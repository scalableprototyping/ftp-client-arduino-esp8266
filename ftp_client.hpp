#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H
#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>

#include <string>
#include <vector>

namespace esp8266_arduino { namespace ftp {

    struct server_ip { IPAddress v; };
    struct server_port { int v; };
    struct user { std::string v; };

    class client {
        public:
            using port_t = int;
            /*
               struct password { std::sting; };
           */

            client(
                    const server_ip,
                    const server_port,
                    const user, 
                    const String password
                  );

            bool upload_file(const String& path,  const String& fileName) const;

        private:
            server_ip _server_ip;
            server_port _server_port;
            user _user;
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
} }

#endif /* FTP_CLIENT_H */
