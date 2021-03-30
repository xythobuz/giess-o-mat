#include <Arduino.h>
#include "Keymatrix.h"
#include "SerialLCD.h"
#include "Statemachine.h"
#include "Plants.h"
#include "config.h"

SerialLCD lcd(9);

Keymatrix keys(4, 3);
int keymatrix_pins[4 + 3] = { 5, 6, 7, 8, 2, 3, 4 };

Plants plants(5, 3, 2);
int valve_pins[5] = { 10, 11, 12, 13, 14 };
int pump_pins[3] = { 15, 16, 17 };
int switch_pins[2] = { 18, 19 };

unsigned long last_input_time = 0;
bool backlight_state = true;

bool doing_multi_input = false;

void write_to_all(const char *a, const char *b,
                  const char *c, const char *d, int num_input) {
    lcd.clear();
    
    if (num_input >= 0) {
        lcd.write(0, a);
        if (num_input >= 1) {
            lcd.write(1, b);
        }
        if (num_input >= 2) {
            lcd.write(2, c);
        }
        if (num_input >= 3) {
            lcd.write(3, d);
        }
        
        lcd.cursor(3);
        doing_multi_input = true;
    } else {
        lcd.write(0, a);
        lcd.write(1, b);
        lcd.write(2, c);
        lcd.write(3, d);
        
        lcd.cursor(0);
        doing_multi_input = false;
    }
    
#ifdef DEBUG_ENABLE_LCD_OUTPUT_ON_SERIAL
    int la = strlen(a);
    int lb = strlen(b);
    int lc = strlen(c);
    int ld = strlen(d);
    
    Serial.println();
    Serial.println(" ----------------------");
    
    Serial.print("| ");
    Serial.print(a);
    if (la < 20) {
        for (int i = 0; i < (20 - la); i++) {
            Serial.print(' ');
        }
    }
    Serial.println(" |");
    
    Serial.print("| ");
    Serial.print(b);
    if (lb < 20) {
        for (int i = 0; i < (20 - lb); i++) {
            Serial.print(' ');
        }
    }
    Serial.println(" |");
    
    Serial.print("| ");
    Serial.print(c);
    if (lc < 20) {
        for (int i = 0; i < (20 - lc); i++) {
            Serial.print(' ');
        }
    }
    Serial.println(" |");
    
    Serial.print("| ");
    Serial.print(d);
    if (ld < 20) {
        for (int i = 0; i < (20 - ld); i++) {
            Serial.print(' ');
        }
    }
    Serial.println(" |");
    
    Serial.println(" ----------------------");
    Serial.println("Please provide keypad input:");
#endif
}

void backspace(void) {
    lcd.write("\b");
}

Statemachine sm(write_to_all, backspace);

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing Giess-o-mat");
    
    keys.setPins(keymatrix_pins);
    plants.setValvePins(valve_pins);
    plants.setPumpPins(pump_pins);
    plants.setSwitchPins(switch_pins, true);

    Serial.println("Setting up LCD, please wait");
    delay(1000); // give LCD some time to boot
    lcd.init();
    
#ifdef DEBUG_WAIT_FOR_SERIAL_CONN
    lcd.write(0, "Waiting for serial");
    lcd.write(1, "connection on debug");
    lcd.write(2, "USB port...");
    
    while (!Serial);
    
    lcd.clear();
#endif
    
    Serial.println("Ready, starting main loop");
    sm.begin();
}

void blink_lcd(int n, int wait = 200) {
    for (int i = 0; i < n; i++) {
        lcd.setBacklight(0);
        delay(wait);
        
        lcd.setBacklight(255);
        if (i < (n - 1))
            delay(wait);
    }
}

void loop() {
    keys.scan();
    while (keys.hasEvent()) {
        auto ke = keys.getEvent();
        if (ke.getType() == Keymatrix::Event::button_down) {
            last_input_time = millis();
            if (!backlight_state) {
                backlight_state = true;
                lcd.setBacklight(255);
                
                // swallow input when used to activate light
                continue;
            }
            
            int n = ke.getNum();
            Serial.print("Got keypad input: \"");
            
            if (n < 0) {
                Serial.print((n == -1) ? '*' : '#');
            } else {
                Serial.print(n);
                
                if (doing_multi_input) {
                    char s[2] = { (char)(n + '0'), '\0' };
                    lcd.write(s);
                }
            }
            
            Serial.println("\"");
            
            blink_lcd(1, 100);
            sm.input(n);
        }
    }
    
#ifdef DEBUG_ENABLE_KEYPAD_INPUT_ON_SERIAL
    if (Serial.available() > 0) {
        last_input_time = millis();
        if (!backlight_state) {
            backlight_state = true;
            lcd.setBacklight(255);
        }
        
        int c = Serial.read();
        if (c == '*') {
            Serial.write(c);
            Serial.write('\n');
            
            if (doing_multi_input) {
                char s[2] = { (char)(c), '\0' };
                lcd.write(s);
            }
            
            sm.input(-1);
        } else if  (c == '#') {
            Serial.write(c);
            Serial.write('\n');
            
            if (doing_multi_input) {
                char s[2] = { (char)(c), '\0' };
                lcd.write(s);
            }
            
            sm.input(-2);
        } else if  (c == '\n') {
            Serial.write('#');
            Serial.write('\n');
            
            if (doing_multi_input) {
                char s[2] = { '#', '\0' };
                lcd.write(s);
            }
            
            sm.input(-2);
        } else if (c == '\b') {
            Serial.write(c);
            sm.input(-1);
        } else if ((c >= '0') && (c <= '9')) {
            Serial.write(c);
            if (!doing_multi_input) {
                Serial.write('\n');
            }
            
            if (doing_multi_input) {
                char s[2] = { (char)(c), '\0' };
                lcd.write(s);
            }
            
            sm.input(c - '0');
        }
    }
#endif
    
    sm.act();
    
    if (backlight_state && (millis() >= (last_input_time + DISPLAY_BACKLIGHT_TIMEOUT))) {
        backlight_state = false;
        lcd.setBacklight(0);
    }
}
