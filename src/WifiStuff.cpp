#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#elif defined(ARDUINO_ARCH_ESP32)

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#endif

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#include "wifi.h"
#include "config.h"
#include "SimpleUpdater.h"
#include "WifiStuff.h"

#define BUILTIN_LED_PIN 1

UPDATE_WEB_SERVER server(80);
SimpleUpdater updater;
unsigned long last_server_handle_time = 0;
unsigned long last_led_blink_time = 0;

String message_buffer_a;
String message_buffer_b;
String message_buffer_c;
String message_buffer_d;

void wifi_set_message_buffer(String a, String b, String c, String d) {
    message_buffer_a = a;
    message_buffer_b = b;
    message_buffer_c = c;
    message_buffer_d = d;
}

void handleRoot() {
    String message = F("<html><head>\n");
    message += F("<title>Giess-o-mat</title>\n");
    message += F("</head><body>\n");
    message += F("<h1>Giess-o-mat</h1>\n");
    
    message += F("\n<pre>\n");
    message += message_buffer_a + '\n';
    message += message_buffer_b + '\n';
    message += message_buffer_c + '\n';
    message += message_buffer_d + '\n';
    message += F("\n</pre>\n");
    
    message += F("\n<p>\n");
    message += F("State: ");
    // TODO
    message += F("\n</p>\n");
    
    message += F("\n<p>\n");
    message += F("Version: ");
    message += FIRMWARE_VERSION;
    message += F("\n<br>\n");
    message += F("MAC: ");
    message += WiFi.macAddress();
    message += F("\n</p>\n");

#if defined(ARDUINO_ARCH_ESP8266)
    
    message += F("\n<p>\n");
    message += F("Reset reason: ");
    message += ESP.getResetReason();
    message += F("\n<br>\n");
    message += F("Free heap: ");
    message += String(ESP.getFreeHeap());
    message += F(" (");
    message += String(ESP.getHeapFragmentation());
    message += F("% fragmentation)");
    message += F("\n<br>\n");
    message += F("Free sketch space: ");
    message += String(ESP.getFreeSketchSpace());
    message += F("\n<br>\n");
    message += F("Flash chip real size: ");
    message += String(ESP.getFlashChipRealSize());

    if (ESP.getFlashChipSize() != ESP.getFlashChipRealSize()) {
        message += F("\n<br>\n");
        message += F("WARNING: sdk chip size (");
        message += (ESP.getFlashChipSize());
        message += F(") does not match!");
    }
    
    message += F("\n</p>\n");
    
#elif defined(ARDUINO_ARCH_ESP32)

    message += F("\n<p>\n");
    message += F("Free heap: ");
    message += String(ESP.getFreeHeap() / 1024.0);
    message += F("k\n<br>\n");
    message += F("Free sketch space: ");
    message += String(ESP.getFreeSketchSpace() / 1024.0);
    message += F("k\n<br>\n");
    message += F("Flash chip size: ");
    message += String(ESP.getFlashChipSize() / 1024.0);
    message += F("k\n</p>\n");
    
#endif

    message += F("<p>\n");
    message += F("Try <a href=\"/update\">/update</a> for OTA firmware updates!\n");
    message += F("</p>\n");
    message += F("</body></html>\n");

    server.send(200, "text/html", message);
}

void wifi_setup() {
    // Build hostname string
    String hostname = "giess-o-mat";

#if defined(ARDUINO_ARCH_ESP8266)

    // Connect to WiFi AP
    WiFi.hostname(hostname);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PW);
    while (WiFi.status() != WL_CONNECTED) {
        delay(LED_CONNECT_BLINK_INTERVAL);
        digitalWrite(BUILTIN_LED_PIN, !digitalRead(BUILTIN_LED_PIN));
    }
    
#elif defined(ARDUINO_ARCH_ESP32)

    // Set hostname workaround
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());
    
    // Workaround for WiFi connecting only every 2nd reset
    // https://github.com/espressif/arduino-esp32/issues/2501#issuecomment-513602522
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (info.disconnected.reason == 202) {
            esp_sleep_enable_timer_wakeup(10);
            esp_deep_sleep_start();
            delay(100);
        }
    }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);

    // Connect to WiFi AP
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PW);
    while (WiFi.status() != WL_CONNECTED) {
        delay(LED_CONNECT_BLINK_INTERVAL);
        digitalWrite(BUILTIN_LED_PIN, !digitalRead(BUILTIN_LED_PIN));
    }
    
    // Set hostname workaround
    WiFi.setHostname(hostname.c_str());

#endif

    // Setup HTTP Server
    MDNS.begin(hostname.c_str());
    updater.setup(&server);
    server.on("/", handleRoot);
    server.begin();
    MDNS.addService("http", "tcp", 80);
}

void wifi_run() {
    if ((millis() - last_server_handle_time) >= SERVER_HANDLE_INTERVAL) {
        last_server_handle_time = millis();
        server.handleClient();
#if defined(ARDUINO_ARCH_ESP8266)
        MDNS.update();
#endif
    }

    // blink heartbeat LED
    if ((millis() - last_led_blink_time) >= LED_BLINK_INTERVAL) {
        last_led_blink_time = millis();
        digitalWrite(BUILTIN_LED_PIN, !digitalRead(BUILTIN_LED_PIN));
    }
    
    // reset ESP every 6h to be safe
    if (millis() >= (6 * 60 * 60 * 1000)) {
        ESP.restart();
    }
}

#endif // defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
