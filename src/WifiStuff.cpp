#include <Arduino.h>

#ifdef PLATFORM_ESP

#if defined(ARDUINO_ARCH_ESP8266)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#elif defined(ARDUINO_ARCH_ESP32)

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#endif

#include "wifi.h"
#include "config.h"
#include "config_pins.h"
#include "SimpleUpdater.h"
#include "WifiStuff.h"

UPDATE_WEB_SERVER server(80);
SimpleUpdater updater;
unsigned long last_server_handle_time = 0;

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
    message += F(" ----------------------\n");
    
    message += F("| ");
    message += message_buffer_a;
    for (int i = 0; i < (20 - message_buffer_a.length()); i++) {
        message += ' ';
    }
    message += F(" |\n");
    
    message += F("| ");
    message += message_buffer_b;
    for (int i = 0; i < (20 - message_buffer_b.length()); i++) {
        message += ' ';
    }
    message += F(" |\n");
    
    message += F("| ");
    message += message_buffer_c;
    for (int i = 0; i < (20 - message_buffer_c.length()); i++) {
        message += ' ';
    }
    message += F(" |\n");
    
    message += F("| ");
    message += message_buffer_d;
    for (int i = 0; i < (20 - message_buffer_d.length()); i++) {
        message += ' ';
    }
    message += F(" |\n");
    
    message += F(" ----------------------\n");
    message += F("</pre>\n");
    
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
    Serial.println("WiFi: initializing");
    WiFi.hostname(hostname);
    WiFi.mode(WIFI_STA);
    
    Serial.print("WiFi: connecting");
    WiFi.begin(WIFI_SSID, WIFI_PW);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(LED_CONNECT_BLINK_INTERVAL);
        digitalWrite(BUILTIN_LED_PIN, !digitalRead(BUILTIN_LED_PIN));
    }
    Serial.println();
    
#elif defined(ARDUINO_ARCH_ESP32)

    // Set hostname workaround
    Serial.println("WiFi: set hostname");
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());
    
    // Workaround for WiFi connecting only every 2nd reset
    // https://github.com/espressif/arduino-esp32/issues/2501#issuecomment-513602522
    Serial.println("WiFi: connection work-around");
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (info.disconnected.reason == 202) {
            esp_sleep_enable_timer_wakeup(10);
            esp_deep_sleep_start();
            delay(100);
        }
    }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);

    // Connect to WiFi AP
    Serial.println("WiFi: SSID=" WIFI_SSID);
    Serial.print("WiFi: connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PW);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(LED_CONNECT_BLINK_INTERVAL);
        digitalWrite(BUILTIN_LED_PIN, !digitalRead(BUILTIN_LED_PIN));
    }
    Serial.println();
    
    // Set hostname workaround
    Serial.println("WiFi: set hostname work-around");
    WiFi.setHostname(hostname.c_str());

#endif

    // Setup HTTP Server
    Serial.println("WiFi: initializing HTTP server");
    MDNS.begin(hostname.c_str());
    updater.setup(&server);
    server.on("/", handleRoot);
    server.begin();
    MDNS.addService("http", "tcp", 80);
    
    Serial.println("WiFi: setup done");
}

void wifi_run() {
    if ((millis() - last_server_handle_time) >= SERVER_HANDLE_INTERVAL) {
        last_server_handle_time = millis();
        server.handleClient();
        
#ifdef ARDUINO_ARCH_ESP8266
        MDNS.update();
#endif // ARDUINO_ARCH_ESP8266
    }
    
    // reset ESP every 6h to be safe
    if (millis() >= (6 * 60 * 60 * 1000)) {
        ESP.restart();
    }
}

#endif // PLATFORM_ESP
