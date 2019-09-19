#include <Arduino.h>

#include "ftp_client.hpp"
#include "ftp_credentials.hpp"

namespace esp8266_arduino { namespace ftp { 
    client test_client (
            server_ip{ IPAddress{109, 73, 237, 190} },
            server_port{21},
            user{USER},
            password{PASSWORD}
        );
    }
}

void setup() {
    // Begin serial communication
    Serial.begin(115200);
    Serial.println(F("FTP Client Project"));

    // Connect to Wifi
    String ssid = "UPC0607098";
    String password = "tJj3fvjhz3Rw";
    WiFi.begin(ssid, password);
    Serial.print(F("Connecting"));
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(F("."));
        delay(100);
        yield();
    }
    Serial.print(F("\n"));
    Serial.println(F("Connected!"));

    // Create a test file
    Serial.println(F("Creating test file."));
    SPIFFS.begin();
    fs::File file_handler;
    file_handler = SPIFFS.open("/test.txt", "w");
    file_handler.println(F("This is a test file."));
    file_handler.close();

}

void hold_until_serial_input() {
    while (Serial.available() <= 0) yield(); // Wait for Serial input
    while (Serial.read() > 0) yield(); // Discard Serial input
}

void loop() {
    Serial.println(F("Press enter to upload a test file to FTP server"));
    hold_until_serial_input();

    esp8266_arduino::ftp::test_client.upload_file("/test.txt", "/test/test.txt");

    Serial.print(F("Heap: "));
    Serial.println(ESP.getFreeHeap());

    Serial.print(F("Heap Fragmentation: "));
    Serial.print(ESP.getHeapFragmentation());
    Serial.println("%");
}
