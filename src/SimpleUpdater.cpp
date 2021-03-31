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
        "<html><head>"
        "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
        "<title>SimpleUpdater ESP32</title>"
        "</head><body>"
        "<h1>SimpleUpdater</h1>"
        "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
        "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
        "</form>"
        "<div id='prg'>progress: 0%</div>"
        "<a href=\"/\">Back to Main Page</a>"
        "<script>"
        "$('form').submit(function(e){"
        "e.preventDefault();"
        "var form = $('#upload_form')[0];"
        "var data = new FormData(form);"
        " $.ajax({"
        "url: '/update',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData:false,"
        "xhr: function() {"
        "var xhr = new window.XMLHttpRequest();"
        "xhr.upload.addEventListener('progress', function(evt) {"
        "if (evt.lengthComputable) {"
        "var per = evt.loaded / evt.total;"
        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
        "}"
        "}, false);"
        "return xhr;"
        "},"
        "success:function(d, s) {"
        "console.log('success!')" 
        "},"
        "error: function (a, b, c) {"
        "}"
        "});"
        "});"
        "</script>"
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
