#include "ftp_client.h"

#include <ESP8266WiFi.h>
#include <FS.h>

#include <functional>
#include <string>

bool try_for_n_attempts(int n, std::function<bool()> function) {
    for (int attempts=0; attempts<n; ++attempts) {
        if (function()) return true;
        yield();
        delay(100);
    }
    return false;
}

//TODO: replace string with std::string
ftp_client::ftp_client(
        const IPAddress server_ip, 
        port_t server_port, 
        const String user, 
        const String password
    ) :
        server_ip(server_ip), 
        server_port(server_port), 
        user(user), 
        password(password) 
{};

String ftp_client::format_bytes(size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + "B";
    } else if (bytes < (1024 * 1024)) {
        return String(bytes / 1024.0) + "KB";
    } else if (bytes < (1024 * 1024 * 1024)) {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    } else {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

ftp_client::connection::connection(const IPAddress& ip, port_t port) {
    tcp_client.connect(ip, port);
}

bool ftp_client::connection::println(const String& message) {
    tcp_client.println(message);
}

bool ftp_client::connection::print(const byte_buffer_t& buffer) {
    tcp_client.write(buffer.data(), buffer.size());
}

bool ftp_client::connection::is_connected() {
    return tcp_client.connected();
}

ftp_client::connection::response ftp_client::connection::receive() {
    connection::response response;

    // Check that at least 3 characters (the response code) are available
    bool response_available = try_for_n_attempts(10, [this] { 
        const int response_code_length = 3;
        return (tcp_client.available() >= response_code_length);
    });

    if (!response_available) {
        Serial.println(F("No FTP server response."));
        return response;
    }

    if (tcp_client.available() >= 3) {
        response.code[0] = tcp_client.read();
        response.code[1] = tcp_client.read();
        response.code[2] = tcp_client.read();
    }
    Serial.print(response.code.c_str());

    byte_buffer_t in_buffer;
    in_buffer.reserve(tcp_client.available());
    while (tcp_client.available()) {
        auto in_byte = tcp_client.read();
        Serial.write(in_byte);
        in_buffer.push_back(in_byte);
    }
    response.body = std::string(in_buffer.data(), in_buffer.size());

    return response;
}

void ftp_client::connection::close(){
    tcp_client.stop();
}

ftp_client::connection::~connection(){
    close();
}

std::vector<int> ftp_client::parse_pasv_response(std::string& s) const {
    char *c_str = strdup(s.c_str());
    char *tStr = strtok(c_str, "(");
    std::vector<int> pasv;
    for (int i = 0; i < 6; i++) {
        tStr = strtok(NULL, ",");
        pasv.push_back(atoi(tStr));
        if (tStr == NULL) {
            Serial.println(F("Bad PASV Answer"));
        }
    }
    free(c_str);
    return pasv;
}

bool ftp_client::upload_file(const String& path, const String& fileName) const {

    if (!SPIFFS.exists(path)) {
        Serial.println(F("File doesn't exists."));
        Serial.println(path);
        return 0;
    }

    ftp_client::file_handler file_handler{path, "r"};

    Serial.println(F("Connecting to FTP server..."));

    ftp_client::connection command_connection{server_ip, server_port};
    if (command_connection.is_connected()) {
        Serial.println(F("FTP connection established!"));
    } else {
        Serial.println(F("FTP connection refused."));
        return 0;
    }

    connection::response response;
    
    response = command_connection.receive();
    if (response.code != "220" ) {
        Serial.println(F("Expected 220 response. Error ocurred"));
        return 0;
    }

    command_connection.println(String("USER ") + user + "\n");
    response = command_connection.receive();
    if (response.code != "331" ) {
        Serial.println(F("Expected 331 response. Error ocurred"));
        return 0;
    }

    command_connection.println(String("PASS ") + password + "\n");
    response = command_connection.receive();
    if (response.code != "230" ) {
        Serial.println(F("Expected 331 response. Error ocurred"));
        return 0;
    }

    // Set Image Data Type RFC 959 3.1.1.3
    command_connection.println(String("TYPE I\n"));
    response = command_connection.receive();
    if (response.code.at(0) != '2' ) {
        Serial.println(F("Expected 2xx response. Error ocurred"));
        return 0;
    }

    // Open FTP pasive Data Port
    command_connection.println(String("PASV\n"));
    response = command_connection.receive();
    if (response.code != "227" ) {
        Serial.println(F("Expected 227 response. Error ocurred"));
        return 0;
    }

    std::vector<int> pasv = parse_pasv_response(response.body);
    unsigned int pasv_port, pasv_port_h, pasv_port_l;
    pasv_port_h = pasv.at(4) << 8;
    pasv_port_l = pasv.at(5);
    pasv_port = pasv_port_h | pasv_port_l;

    ftp_client::connection data_connection(server_ip, pasv_port);
    if (data_connection.is_connected()) {
        Serial.println(F("Data connection established"));
    }
    else {
        Serial.println(F("Data connection refused"));
        return 0;
    }

    command_connection.println(String("STOR ") + fileName + "\n");
    response = command_connection.receive();

    // TODO: improve to stl
#define bufSizeFTP 1460
    uint8_t clientBuf[bufSizeFTP];
    size_t clientCount = 0;

    while (file_handler.file.available()) {
        clientBuf[clientCount] = file_handler.file.read();
        clientCount++;
        if (clientCount > (bufSizeFTP - 1)) {
            auto send_buffer = connection::byte_buffer_t(clientBuf, clientBuf + bufSizeFTP);
            data_connection.print(send_buffer);
            clientCount = 0;
            delay(1);
        }
    }
    if (clientCount > 0) {
        auto send_buffer = connection::byte_buffer_t(clientBuf, clientBuf + clientCount);
        data_connection.print(send_buffer);
    }

    data_connection.close();
    response = command_connection.receive();

    command_connection.println(F("QUIT\n"));
    response = command_connection.receive();

    return 1;
}