/*
 * SimpleUpdater.h
 *
 * ESP8266 / ESP32 Environmental Sensor
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <xythobuz@xythobuz.de> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.   Thomas Buck
 * ----------------------------------------------------------------------------
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

