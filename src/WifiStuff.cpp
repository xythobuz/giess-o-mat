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

#include <WebSocketsServer.h>

#include "wifi.h"
#include "config.h"
#include "config_pins.h"
#include "Functionality.h"
#include "SimpleUpdater.h"
#include "WifiStuff.h"

UPDATE_WEB_SERVER server(80);
WebSocketsServer socket = WebSocketsServer(81);
SimpleUpdater updater;
unsigned long last_server_handle_time = 0;
unsigned long last_websocket_update_time = 0;

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

void wifi_schedule_websocket(void) {
    last_websocket_update_time = 0;
}

void wifi_send_websocket(void) {
    String a = message_buffer_a ;
    String b = message_buffer_b;
    String c = message_buffer_c;
    String d = message_buffer_d;
    
    a.replace("\"", "'");
    b.replace("\"", "'");
    c.replace("\"", "'");
    d.replace("\"", "'");
    
    String ws = "{\n";
    
    ws += "\"a\": \"" + a + "\",\n";
    ws += "\"b\": \"" + b + "\",\n";
    ws += "\"c\": \"" + c + "\",\n";
    ws += "\"d\": \"" + d + "\",\n";
    
    ws += "\"state\": \"" + String(control_state_name()) + "\",\n";
    
    ws += F("\"valves\": [ ");
    for (int i = 0; i < VALVE_COUNT; i++) {
        ws += "\"";
        ws += get_plants()->getValves()->getPin(i) ? "1" : "0";
        ws += "\"";
        
        if (i < (VALVE_COUNT - 1)) {
            ws += ", ";
        }
    }
    ws += " ],\n";
    
    ws += F("\"pumps\": [ ");
    for (int i = 0; i < PUMP_COUNT; i++) {
        ws += "\"";
        ws += get_plants()->getPumps()->getPin(i) ? "1" : "0";
        ws += "\"";
        
        if (i < (PUMP_COUNT - 1)) {
            ws += ", ";
        }
    }
    ws += " ],\n";
    
    ws += F("\"switches\": [ ");
    for (int i = 0; i < SWITCH_COUNT; i++) {
        ws += "\"";
        ws += get_plants()->getSwitches()->getPin(i) ? "1" : "0";
        ws += "\"";
        
        if (i < (SWITCH_COUNT - 1)) {
            ws += ", ";
        }
    }
    ws += " ],\n";
    
    ws += "\"switchstate\": \"";
    Plants::Waterlevel wl = get_plants()->getWaterlevel();
    if (wl == Plants::empty) {
        ws += F("tank empty");
    } else if (wl == Plants::inbetween) {
        ws += F("tank half-filled");
    } else if (wl == Plants::full) {
        ws += F("tank full");
    } else {
        ws += F("invalid sensor state");
    }
    ws += "\"\n";
    ws += "}";
    
    socket.broadcastTXT(ws);
}

void handleRoot() {
    String message = F("<!DOCTYPE html>\n");
    message += F("<html><head>\n");
    message += F("<meta charset='utf-8'/>\n");
    message += F("<meta name='viewport' content='width=device-width, initial-scale=1'/>\n");
    message += F("<title>Gieß-o-mat</title>\n");
    
    message += F("<style type='text/css'>\n");
    message += F(".container {\n");
    message += F("display: flex;\n");
    message += F("}\n");
    
    message += F(".ui {\n");
    message += F("width: max-content;\n");
    message += F("height: max-content;\n");
    message += F("margin-right: 1em;\n");
    message += F("padding: 0 1.0em;\n");
    message += F("border: 1px dashed black;\n");
    message += F("}\n");
    
    message += F(".io {\n");
    message += F("width: max-content;\n");
    message += F("height: max-content;\n");
    message += F("margin-right: 1em;\n");
    message += F("padding: 0.8em 1.0em;\n");
    message += F("border: 1px dashed black;\n");
    message += F("font-family: monospace;\n");
    message += F("}\n");
    
    message += F(".switch {\n");
    message += F("width: max-content;\n");
    message += F("border: 1px solid black;\n");
    message += F("border-radius: 50%;\n");
    message += F("padding: 2em;\n");
    message += F("margin: 1em;\n");
    message += F("}\n");
    
    message += F(".valve {\n");
    message += F("width: max-content;\n");
    message += F("border: 1px solid black;\n");
    message += F("border-radius: 50%;\n");
    message += F("padding: 2em;\n");
    message += F("margin: 1em;\n");
    message += F("}\n");
    
    message += F(".pump {\n");
    message += F("width: max-content;\n");
    message += F("border: 1px solid black;\n");
    message += F("border-radius: 50%;\n");
    message += F("padding: 2em;\n");
    message += F("margin: 1em;\n");
    message += F("}\n");
    
    message += F(".info {\n");
    message += F("width: max-content;\n");
    message += F("height: max-content;\n");
    message += F("padding: 0 1.0em;\n");
    message += F("border: 1px dashed black;\n");
    message += F("font-family: monospace;\n");
    message += F("}\n");
    
    message += F(".pad {\n");
    message += F("background: black;\n");
    message += F("border: 3px solid black;\n");
    message += F("border-radius: 20px;\n");
    message += F("width: max-content;\n");
    message += F("padding: 1.5em;\n");
    message += F("margin-left: auto;\n");
    message += F("margin-right: auto;\n");
    message += F("}\n");
    
    message += F(".pad input {\n");
    message += F("background: #fff0cf;\n");
    message += F("border-radius: 6px;\n");
    message += F("font-weight: bold;\n");
    message += F("font-family: monospace;\n");
    message += F("font-size: 1.2em;\n");
    message += F("padding: 0.5em 1em;\n");
    message += F("margin: 0.5em;\n");
    message += F("}\n");
    
    // https://codepen.io/hawkz/pres/RpPaGK
    message += F(".lcd {\n");
    //message += F("background: #9ea18c;\n");
    message += F("background: #9ed18c;\n");
    message += F("border: 5px solid black;\n");
    message += F("border-radius: 10px;\n");
    message += F("width: max-content;\n");
    message += F("padding: 0.65em 1em;\n");
    message += F("box-shadow: inset 0 0 5px 5px rgba(0,0,0,.1);\n");
    message += F("font-weight: bold;\n");
    message += F("font-family: monospace;\n");
    message += F("letter-spacing: 0.1em;\n");
    message += F("font-size: 1.2em;\n");
    message += F("line-height: 160%;\n");
    message += F("color: #21230e;\n");
    message += F("text-shadow: -1px 2px 1px rgba(0,0,0,.1);\n");
    message += F("margin-left: auto;\n");
    message += F("margin-right: auto;\n");
    message += F("}\n");
    
    message += F("#state {\n");
    message += F("text-align: center;\n");
    message += F("}\n");
    message += F("</style>\n");
    
    message += F("</head><body>\n");
    message += F("<h1>Gieß-o-mat</h1>\n");
    
    message += F("<div class='container'>\n");
    message += F("<div class='ui'>\n");
    message += F("<pre class='lcd'>\n");
    message += message_buffer_a + '\n';
    message += message_buffer_b + '\n';
    message += message_buffer_c + '\n';
    message += message_buffer_d + '\n';
    message += F("</pre>\n");
    
    message += F("<form class='pad'>\n");
    message += F("<input type='button' value='1'>");
    message += F("<input type='button' value='2'>");
    message += F("<input type='button' value='3'>");
    message += F("<br>\n");
    message += F("<input type='button' value='4'>");
    message += F("<input type='button' value='5'>");
    message += F("<input type='button' value='6'>");
    message += F("<br>\n");
    message += F("<input type='button' value='7'>");
    message += F("<input type='button' value='8'>");
    message += F("<input type='button' value='9'>");
    message += F("<br>\n");
    message += F("<input type='button' value='*'>");
    message += F("<input type='button' value='0'>");
    message += F("<input type='button' value='#'>");
    message += F("</form>\n");
    
    message += F("<p id='state'>\n");
    message += F("State: ");
    message += control_state_name();
    message += F("</p></div>\n");
    
    message += F("<div class='io'>\n");
    message += F("Switches: <span id='switchstate'>");
    
    Plants::Waterlevel wl = get_plants()->getWaterlevel();
    if (wl == Plants::empty) {
        message += F("tank empty");
    } else if (wl == Plants::inbetween) {
        message += F("tank half-filled");
    } else if (wl == Plants::full) {
        message += F("tank full");
    } else {
        message += F("invalid sensor state");
    }
    message += F("</span>");
    
    message += F("<div class='container'>\n");
    for (int i = 0; i < SWITCH_COUNT; i++) {
        message += F("<div class='switch' style='background-color: ");
        if (get_plants()->getSwitches()->getPin(i)) {
            message += F("red");
        } else {
            message += F("green");
        }
        message += F(";'>S");
        message += String(i + 1);
        message += F("</div>");
    }
    message += F("</div><hr>\n");
    
    message += F("Valves:\n");
    message += F("<div class='container'>\n");
    for (int i = 0; i < VALVE_COUNT; i++) {
        message += F("<div class='valve' style='background-color: ");
        if (get_plants()->getValves()->getPin(i)) {
            message += F("red");
        } else {
            message += F("green");
        }
        message += F(";'>V");
        message += String(i + 1);
        message += F("</div>");
    }
    message += F("</div><hr>\n");
    
    message += F("Pumps:\n");
    message += F("<div class='container'>\n");
    for (int i = 0; i < PUMP_COUNT; i++) {
        message += F("<div class='pump' style='background-color: ");
        if (get_plants()->getPumps()->getPin(i)) {
            message += F("red");
        } else {
            message += F("green");
        }
        message += F(";'>P");
        message += String(i + 1);
        message += F("</div>");
    }
    message += F("</div><hr>\n");
    message += F("Green means valve is closed / pump is off / switch is not submersed.\n");
    message += F("<br>\n");
    message += F("Red means valve is open / pump is running / switch is submersed.</div>\n");
    
    message += F("<div class='info'><p>\n");
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

    message += F("<p>Try <a href='/update'>/update</a> for OTA firmware updates!</p>\n");
    message += F("<p>Made by <a href='https://xythobuz.de'>xythobuz</a></p>\n");
    message += F("<p><a href='https://git.xythobuz.de/thomas/giess-o-mat'>Project Repository</a></p>\n");
    message += F("</div>\n");
    message += F("</div></body>\n");
    
    message += F("<script type='text/javascript'>\n");
    message += F("var socket = new WebSocket('ws://' + window.location.hostname + ':81');\n");
    message += F("socket.onmessage = function(e) {\n");
    message += F(    "var msg = JSON.parse(e.data);\n");
    message += F(    "var str = msg.a + '\\n' + msg.b + '\\n' + msg.c + '\\n' + msg.d;\n");
    message += F(    "console.log(str);\n");
    message += F(    "var lcd = document.getElementsByClassName('lcd');\n");
    message += F(    "lcd[0].innerHTML = str;\n");
    message += F(    "var state = document.getElementById('state');\n");
    message += F(    "state.innerHTML = \"State: \" + msg.state;\n");
    
    message += F(    "for (let i = 0; i < ");
    message += String(VALVE_COUNT);
    message += F("; i++) {\n");
    message += F(       "var valves = document.getElementsByClassName('valve');\n");
    message += F(       "if (msg.valves[i] == '0') {\n");
    message += F(           "valves[i].style = 'background-color: green;';\n");
    message += F(       "} else {\n");
    message += F(           "valves[i].style = 'background-color: red;';\n");
    message += F(       "}\n");
    message += F(    "}\n");
    
    message += F(    "for (let i = 0; i < ");
    message += String(PUMP_COUNT);
    message += F("; i++) {\n");
    message += F(       "var pumps = document.getElementsByClassName('pump');\n");
    message += F(       "if (msg.pumps[i] == '0') {\n");
    message += F(           "pumps[i].style = 'background-color: green;';\n");
    message += F(       "} else {\n");
    message += F(           "pumps[i].style = 'background-color: red;';\n");
    message += F(       "}\n");
    message += F(    "}\n");
    
    message += F(    "for (let i = 0; i < ");
    message += String(SWITCH_COUNT);
    message += F("; i++) {\n");
    message += F(       "var switches = document.getElementsByClassName('switch');\n");
    message += F(       "if (msg.switches[i] == '0') {\n");
    message += F(           "switches[i].style = 'background-color: green;';\n");
    message += F(       "} else {\n");
    message += F(           "switches[i].style = 'background-color: red;';\n");
    message += F(       "}\n");
    message += F(    "}\n");
    
    message += F(    "var switchstate = document.getElementById('switchstate');\n");
    message += F(    "switchstate.innerHTML = msg.switchstate;\n");
    message += F("};\n");
    
    message += F("var buttons = document.getElementsByTagName('input');\n");
    message += F("for (let i = 0; i < buttons.length; i++) {\n");
    message += F(    "buttons[i].addEventListener('click', updateButton);\n");
    message += F("}\n");
    message += F("function updateButton() {\n");
    message += F(    "socket.send(this.value);\n");
    message += F("}\n");
    message += F("</script>\n");
    message += F("</html>\n");

    server.send(200, "text/html", message);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if ((type != WStype_TEXT) || (length != 1)) {
        return;
    }
    
    char c = payload[0];
    if ((c >= '0') && (c <= '9')) {
        control_act_input(c - '0');
    } else if (c == '*') {
        control_act_input(-1);
    } else if (c == '#') {
        control_act_input(-2);
    }
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
    
    socket.begin();
    socket.onEvent(webSocketEvent);
    
    Serial.println("WiFi: setup done");
}

void wifi_run() {
    if ((millis() - last_server_handle_time) >= SERVER_HANDLE_INTERVAL) {
        last_server_handle_time = millis();
        server.handleClient();
        socket.loop();
        
#ifdef ARDUINO_ARCH_ESP8266
        MDNS.update();
#endif // ARDUINO_ARCH_ESP8266
    }
    
    if ((millis() - last_websocket_update_time) >= WEBSOCKET_UPDATE_INTERVAL) {
        last_websocket_update_time = millis();
        wifi_send_websocket();
    }
    
    // reset ESP every 6h to be safe
    if (millis() >= (6 * 60 * 60 * 1000)) {
        ESP.restart();
    }
}

#endif // PLATFORM_ESP
