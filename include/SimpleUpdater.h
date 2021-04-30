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

#ifndef __ESP_SIMPLE_UPDATER__
#define __ESP_SIMPLE_UPDATER__

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#define UPDATE_WEB_SERVER ESP8266WebServer
#elif defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#define UPDATE_WEB_SERVER WebServer
#endif

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

class SimpleUpdater {
public:
    SimpleUpdater(String _uri = String("/update"));
    void setup(UPDATE_WEB_SERVER *_server);
    
private:

#if defined(ARDUINO_ARCH_ESP8266)
    
    ESP8266HTTPUpdateServer updateServer;

#elif defined(ARDUINO_ARCH_ESP32)
    
    void get(void);
    void postResult(void);
    void postUpload(void);
    
#endif
    
    String uri;
    UPDATE_WEB_SERVER *server;
};

#endif

#endif // __ESP_SIMPLE_UPDATER__

