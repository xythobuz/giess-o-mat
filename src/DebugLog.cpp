/*
 * Copyright (c) 2021 Thomas Buck <thomas@xythobuz.de>
 *
 * This file is part of Giess-o-mat.
 *
 * Giess-o-mat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Giess-o-mat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giess-o-mat.  If not, see <https://www.gnu.org/licenses/>.
 */

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
