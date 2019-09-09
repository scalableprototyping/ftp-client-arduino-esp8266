#pragma once

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <FS.h>

#include <string>
#include <vector>

class ftp_client {
    public:
        using port_t = int;

        ftp_client(
                const IPAddress server_ip, 
                port_t server_port, 
                const String user, 
                const String password
                );

        bool upload_file(const String& path,  const String& fileName) const;

    private:
        IPAddress server_ip;
        port_t server_port;
        String user;
        String password;

        std::vector<int> parse_pasv_response(std::string& s) const;

        class connection {
            public:
                using byte_buffer_t = std::vector<char>;

                struct response {
                    std::string code = "000";
                    std::string body;
                };

                connection(const IPAddress& ip, port_t port);
                response receive();
                bool println(const String& message);
                bool print(const byte_buffer_t& buffer);
                bool is_connected();
                void close();
                ~connection(); 

            private:
                WiFiClient tcp_client;
        };

        class file_handler {
            public:
                file_handler(const String& path, const char* mode) {
                    file = SPIFFS.open(path, mode);
                }
                ~file_handler() {
                    file.close();
                }
                fs::File file;
        };
};

String format_bytes(size_t bytes);

