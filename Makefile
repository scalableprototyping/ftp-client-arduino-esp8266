ESP_ROOT := $(HOME)/programs/arduino/esp8266
ESP_LIBS := $(ESP_ROOT)/libraries

FLASH_DEF := 4M1M

SKETCH = ftp_client.ino

UPLOAD_PORT = /dev/ttyUSB0
BOARD = nodemcu

LIBS := $(ESP_LIBS)/SoftwareSerial/src \
		$(ESP_LIBS)/ESP8266WiFi \
		$(ESP_LIBS)/ESP8266WebServer \
		$(ESP_LIBS)/ESP8266HTTPClient \
		$(ESP_LIBS)/EEPROM \
		$(ESP_LIBS)/GDBStub \

BUILD_EXTRA_FLAGS := -O3 -ggdb -std=c++11

include $(HOME)/programs/arduino/makeEspArduino/makeEspArduino.mk
