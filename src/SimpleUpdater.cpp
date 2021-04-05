/*
 * SimpleUpdater.cpp
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

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPUpdateServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <Update.h>
#endif

#include "SimpleUpdater.h"

#if defined(ARDUINO_ARCH_ESP32)

void SimpleUpdater::get(void) {
    String uploadPage = F(
        "<!DOCTYPE html>\n"
        "<html><head>\n"
        "<meta charset='utf-8'/>\n"
        "<meta name='viewport' content='width=device-width, initial-scale=1'/>\n"
        "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>\n"
        "<title>SimpleUpdater ESP32</title>\n"
        "</head><body>\n"
        
        "<h1>SimpleUpdater</h1>\n"
        "<p>Select the update file. If you have built this project with PlatformIO, you can find a firmware.bin in the .pio folder.</p>\n"
        "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>\n"
        "<input type='file' name='update' accept='.bin'>\n"
        "<input type='submit' value='Update'>\n"
        "</form><br>\n"
        "<div id='prg'>progress: 0%</div>\n"
        "<p>After the update is finished, you will automatically be redirected to the main page.</p>\n"
        "<a href=\"/\">Back to Main Page</a>\n"
        
        "<script>\n"
        "$('form').submit(function(e){\n"
            "e.preventDefault();\n"
            "var form = $('#upload_form')[0];\n"
            "var data = new FormData(form);\n"
            " $.ajax({\n"
                "url: '/update',\n"
                "type: 'POST',\n"
                "data: data,\n"
                "contentType: false,\n"
                "processData:false,\n"
                "xhr: function() {\n"
                    "var xhr = new window.XMLHttpRequest();\n"
                    "xhr.upload.addEventListener('progress', function(evt) {\n"
                        "if (evt.lengthComputable) {\n"
                            "var per = evt.loaded / evt.total;\n"
                            "$('#prg').html('progress: ' + Math.round(per*100) + '%');\n"
                        "}\n"
                    "}, false);\n"
                    "return xhr;\n"
                "},\n"
                "success: function(d, s) {\n"
                    "$('#prg').html('progress: success! redirecting...');\n"
                    "setTimeout(function() {\n"
                        "window.location.href = '/';\n"
                    "}, 3000);\n"
                "},\n"
                "error: function(a, b, c) {\n"
                    "$('#prg').html('progress: finished! redirecting...');\n"
                    "setTimeout(function() {\n"
                        "window.location.href = '/';\n"
                    "}, 1000);\n"
                "}\n"
            "});\n"
        "});\n"
        "</script>\n"
        "</body></html>"
    );
    
    server->send(200, "text/html", uploadPage);
}

void SimpleUpdater::postResult(void) {
    server->sendHeader("Connection", "close");
    server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void SimpleUpdater::postUpload(void) {
    HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        // start with max available size
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // flashing firmware to ESP
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        // true to set the size to the current progress
        if (Update.end(true)) {
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

#endif

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

void SimpleUpdater::setup(UPDATE_WEB_SERVER *_server) {
    if (_server == NULL) {
        return;
    }
    
    server = _server;
    
#if defined(ARDUINO_ARCH_ESP8266)
    
    updateServer.setup(server);
    
#elif defined(ARDUINO_ARCH_ESP32)
    
    server->on(uri.c_str(), HTTP_POST, [this]() {
        postResult();
    }, [this]() {
        postUpload();
    });
    
    server->on(uri.c_str(), HTTP_GET, [this]() {
        get();
    });
    
#endif
}

SimpleUpdater::SimpleUpdater(String _uri) {
    uri = _uri;
    server = NULL;
}

#endif
