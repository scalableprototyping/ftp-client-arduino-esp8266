
namespace esp8266_arduino {

    class tcp_connection {
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

}

