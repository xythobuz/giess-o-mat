#include <Arduino.h>

#include "Functionality.h"
#include "WifiStuff.h"
#include "DebugLog.h"
#include "config.h"
#include "config_pins.h"

unsigned long last_led_blink_time = 0;

void setup() {
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    
    Serial.begin(115200);
    debug.println("Initializing Giess-o-mat");
    debug.println("Version: " FIRMWARE_VERSION);

#ifdef FUNCTION_UI
    debug.println("Initializing UI");
    ui_setup();
#endif // FUNCTION_UI
    
#ifdef FUNCTION_CONTROL
    debug.println("Initializing Control");
    control_setup();
#endif // FUNCTION_CONTROL
    
#ifdef PLATFORM_ESP
    debug.println("Initializing WiFi");
    wifi_setup();
#endif // PLATFORM_ESP
    
    debug.println("Ready, starting main loop");
    digitalWrite(BUILTIN_LED_PIN, LOW);
    
#ifdef FUNCTION_CONTROL
    
#ifndef FUNCTION_UI
    // give ui unit some time to initialize
    debug.println("Waiting for UI to boot");
    delay(3000);
#endif // ! FUNCTION_UI
    
    debug.println("Starting state machine");
    control_begin();
    
#endif // FUNCTION_CONTROL
}

void loop() {
#ifdef PLATFORM_ESP
    wifi_run();
#endif // PLATFORM_ESP

#ifdef FUNCTION_UI
    ui_run();
#endif // FUNCTION_UI
    
#ifdef FUNCTION_CONTROL
    control_run();
#endif // FUNCTION_CONTROL

    // blink heartbeat LED
    if ((millis() - last_led_blink_time) >= LED_BLINK_INTERVAL) {
        last_led_blink_time = millis();
        digitalWrite(BUILTIN_LED_PIN, !digitalRead(BUILTIN_LED_PIN));
    }
}
