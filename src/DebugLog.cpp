#include <Arduino.h>

#ifdef PLATFORM_ESP
#include "WifiStuff.h"
#endif // PLATFORM_ESP

#include "DebugLog.h"

DebugLog debug;

#ifdef PLATFORM_ESP

String DebugLog::getBuffer(void) {
    String r;
    for (unsigned int i = 0; i < buffer.size(); i++) {
        r += buffer[i];
    }
    return r;
}

void DebugLog::addToBuffer(String s) {
    for (unsigned int i = 0; i < s.length(); i++) {
        buffer.push(s[i]);
    }
}

#endif // PLATFORM_ESP

void DebugLog::sendToTargets(String s) {
    Serial.print(s);
    
#ifdef PLATFORM_ESP
    s = "log:" + s;
    wifi_send_websocket(s);
#endif // PLATFORM_ESP
}

void DebugLog::print(String s) {
#ifdef PLATFORM_ESP
    addToBuffer(s);
#endif // PLATFORM_ESP
    
    sendToTargets(s);
}

void DebugLog::print(int n) {
    print(String(n));
}

void DebugLog::println(void) {
    print(String('\n'));
}

void DebugLog::println(String s) {
    s += '\n';
    print(s);
}

void DebugLog::println(int n) {
    println(String(n));
}
