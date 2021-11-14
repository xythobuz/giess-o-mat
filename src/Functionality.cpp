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

#include "config.h"
#include "config_pins.h"
#include "DebugLog.h"
#include "Functionality.h"

#ifdef FUNCTION_UI

#ifndef FUNCTION_CONTROL

#include <Wire.h>
#include <CircularBuffer.h>

CircularBuffer<int, I2C_BUF_SIZE> keybuffer;
String linebuffer[4];

bool sm_is_idle(void) {
    return true;
}

#endif // ! FUNCTION_CONTROL

#include "SerialLCD.h"
#include "Keymatrix.h"

SerialLCD lcd(SERIAL_LCD_TX_PIN);

Keymatrix keys(KEYMATRIX_ROWS, KEYMATRIX_COLS);
int keymatrix_pins[KEYMATRIX_ROWS + KEYMATRIX_COLS] = { KEYMATRIX_ROW_PINS, KEYMATRIX_COL_PINS };

unsigned long last_input_time = 0;
bool backlight_state = true;
bool doing_multi_input = false;

#endif // FUNCTION_UI

#ifdef FUNCTION_CONTROL

#ifndef FUNCTION_UI
#include <Wire.h>
#endif // ! FUNCTION_UI

#include "Plants.h"
#include "Statemachine.h"
#include "WifiStuff.h"

Plants plants(VALVE_COUNT, PUMP_COUNT, SWITCH_COUNT, AUX_COUNT);
int valve_pins[VALVE_COUNT] = { VALVE_PINS };
int pump_pins[PUMP_COUNT] = { PUMP_PINS };
int switch_pins[SWITCH_COUNT] = { SWITCH_PINS };
int aux_pins[AUX_COUNT] = { AUX_PINS };

Statemachine sm(write_to_all, backspace);

bool sm_is_idle(void) {
    return sm.isIdle();
}

#endif // FUNCTION_CONTROL

// ----------------------------------------------------------------------------

void write_lcd_to_serial(const char *a, const char *b,
                  const char *c, const char *d) {
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
#endif // DEBUG_ENABLE_LCD_OUTPUT_ON_SERIAL
}

void handle_input(int n) {
#ifdef FUNCTION_CONTROL
    
    sm.input(n);
    
#else
    
    keybuffer.push(n);
    
#endif // FUNCTION_CONTROL
}

void input_serial(void) {
#ifdef DEBUG_ENABLE_KEYPAD_INPUT_ON_SERIAL
    if (Serial.available() > 0) {
        
#ifdef FUNCTION_UI
        last_input_time = millis();
        if (!backlight_state) {
            backlight_state = true;
            lcd.setBacklight(255);
        }
#endif // FUNCTION_UI
        
        int c = Serial.read();
        if (c == '*') {
            Serial.write(c);
            Serial.write('\n');
            
#ifdef FUNCTION_UI
            if (doing_multi_input) {
                char s[2] = { (char)(c), '\0' };
                lcd.write(s);
            }
#endif // FUNCTION_UI
            
            handle_input(-1);
        } else if  (c == '#') {
            Serial.write(c);
            Serial.write('\n');
            
#ifdef FUNCTION_UI
            if (doing_multi_input) {
                char s[2] = { (char)(c), '\0' };
                lcd.write(s);
            }
#endif // FUNCTION_UI
            
            handle_input(-2);
        } else if  (c == '\n') {
            Serial.write('#');
            Serial.write('\n');
            
#ifdef FUNCTION_UI
            if (doing_multi_input) {
                char s[2] = { '#', '\0' };
                lcd.write(s);
            }
#endif // FUNCTION_UI
            
            handle_input(-2);
        } else if (c == '\b') {
            Serial.write(c);
            handle_input(-1);
        } else if ((c >= '0') && (c <= '9')) {
            Serial.write(c);
            
#ifdef FUNCTION_UI
            if (!doing_multi_input) {
                Serial.write('\n');
            }
            
            if (doing_multi_input) {
                char s[2] = { (char)(c), '\0' };
                lcd.write(s);
            }
#endif // FUNCTION_UI
            
            handle_input(c - '0');
        }
    }
#endif // DEBUG_ENABLE_KEYPAD_INPUT_ON_SERIAL
}

// ----------------------------------------------------------------------------

#ifdef FUNCTION_UI

#ifndef FUNCTION_CONTROL

void ui_i2c_request(void) {
    if (keybuffer.isEmpty()) {
        Wire.write(252);
        return;
    }
    
    debug.print("ui_i2c_request: ");
    
    while (!keybuffer.isEmpty()) {
        int n = keybuffer.shift();
        
        if (n == -1) {
            n = 254;
        } else if (n == -2) {
            n = 253;
        } else if ((n < 0) || (n > 9)) {
            continue;
        }
        
        debug.print(n);
        debug.print(", ");
        
        Wire.write(n);
    }
    
    debug.println();
}

void ui_i2c_receive(int count) {
    char buff[I2C_BUF_SIZE];
    for (int i = 0; i < I2C_BUF_SIZE; i++) {
        buff[i] = 0;
    }
    
    for (int i = 0; (i < count) && (Wire.available()); i++) {
        buff[i] = Wire.read();
    }
    
    if (count <= 0) {
        debug.println("ui_i2c_receive: count is 0");
        return;
    }
    
    if (buff[0] == 0x01) {
        if (count < 3) {
            debug.println("ui_i2c_receive: blink lcd too short");
            return;
        }
        
        int n = buff[1];
        int wait = buff[2] * 10;
        
        debug.println("ui_i2c_receive: blink lcd command");
        blink_lcd(n, wait);
    } else if (buff[0] == 0x02) {
        debug.println("ui_i2c_receive: backspace command");
        backspace();
    } else if (buff[0] == 0x03) {
        if (count < 3) {
            debug.println("ui_i2c_receive: display far too short");
            return;
        }
        
        int line = buff[1];
        int len = buff[2];
        String s;
        for (int i = 0; i < len; i++) {
            s += buff[3 + i];
        }
        
        debug.println("ui_i2c_receive: display command");
        linebuffer[line] = s;
    } else if (buff[0] == 0x04) {
        if (count < 2) {
            debug.println("ui_i2c_receive: num input too short");
            return;
        }
        
        int8_t num_input = buff[1];
        
        debug.println("ui_i2c_receive: num input");
        write_to_all(linebuffer[0].c_str(), linebuffer[1].c_str(),
                     linebuffer[2].c_str(), linebuffer[3].c_str(),
                     num_input);
    } else {
        debug.println("ui_i2c_receive: unknown command");
        return;
    }
}

#endif // ! FUNCTION_CONTROL

void ui_setup(void) {
    keys.setPins(keymatrix_pins);
    
    debug.println("Setting up LCD, please wait");
    delay(1000); // give LCD some time to boot
    lcd.init();
    lcd.disableSplash();
    
#ifdef DEBUG_WAIT_FOR_SERIAL_CONN
    lcd.write(0, "Waiting for serial");
    lcd.write(1, "connection on debug");
    lcd.write(2, "USB port...");
    while (!Serial);
    lcd.clear();
#endif // DEBUG_WAIT_FOR_SERIAL_CONN
    
#ifndef FUNCTION_CONTROL
    debug.println("Initializing I2C Slave");
    Wire.begin(OWN_I2C_ADDRESS);
    Wire.onReceive(ui_i2c_receive);
    Wire.onRequest(ui_i2c_request);
    
    String a = String("- Giess-o-mat V") + FIRMWARE_VERSION + String(" -");
    String b = String("    Address 0x") + String(OWN_I2C_ADDRESS, HEX) + String("    ");
    
    lcd.write(0, a.c_str());
    lcd.write(1, "    I2C UI Panel    ");
    lcd.write(2, "Waiting for data....");
    lcd.write(3, b.c_str());
#endif // ! FUNCTION_CONTROL
}

void input_keypad(void) {
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
            debug.print("Got keypad input: \"");
            
            if (n < 0) {
                debug.print((n == -1) ? '*' : '#');
            } else {
                debug.print(n);
                
                if (doing_multi_input) {
                    char s[2] = { (char)(n + '0'), '\0' };
                    lcd.write(s);
                }
            }
            
            debug.println("\"");
            
            blink_lcd(1, 100);
            handle_input(n);
        }
    }
}

void ui_run(void) {
    input_keypad();
    input_serial();
    
    if (backlight_state && (millis() >= (last_input_time + DISPLAY_BACKLIGHT_TIMEOUT))) {
        backlight_state = false;
        lcd.setBacklight(0);
    }
}

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
    
    write_lcd_to_serial(a, b, c, d);
}

void backspace(void) {
    lcd.write("\b");
}

void blink_lcd(int n, int wait) {
    for (int i = 0; i < n; i++) {
        lcd.setBacklight(0);
        delay(wait);
        
        lcd.setBacklight(255);
        if (i < (n - 1))
            delay(wait);
    }
}

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

const char *control_state_name(void) {
    return sm.getStateName();
}

void control_act_input(int n) {
    sm.input(n);
}

Plants *get_plants(void) {
    return &plants;
}

void control_setup(void) {
    plants.setValvePins(valve_pins);
    plants.setPumpPins(pump_pins);
    plants.setSwitchPins(switch_pins, true);
    plants.setAuxPins(aux_pins);
    
#ifndef FUNCTION_UI
    
    debug.println("Initializing I2C Master");
    Wire.setClock(I2C_BUS_SPEED);
    
#if defined(I2C_SDA_PIN) && defined(I2C_SCL_PIN)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
#else
    Wire.begin();
#endif // defined(I2C_SDA_PIN) && defined(I2C_SCL_PIN)
    
#ifdef DEBUG_WAIT_FOR_SERIAL_CONN
    debug.println("Wait for Serial");
    while (!Serial);
#endif // DEBUG_WAIT_FOR_SERIAL_CONN
    
#endif // ! FUNCTION_UI
}

void control_begin(void) {
    sm.begin();
}

void control_run(void) {
#ifndef FUNCTION_UI
    
    Wire.requestFrom(OWN_I2C_ADDRESS, I2C_BUF_SIZE);
    while (Wire.available()) {
        int c = Wire.read();
        
        if (((c >= 0) && (c <= 9)) || (c == 254) || (c == 253)) {
            debug.print("control_run: got input '");
            debug.print(c);
            debug.println("'");
            
            if (c == 254) {
                c = -1;
            } else if (c == 253) {
                c = -2;
            }
        
            sm.input(c);
        }
    }
    
    input_serial();

#endif // ! FUNCTION_UI
    
    sm.act();
}

#ifndef FUNCTION_UI

void blink_lcd(int n, int wait) {
    debug.println("blink_lcd i2c");
    
    Wire.beginTransmission(OWN_I2C_ADDRESS);
    Wire.write(0x01); // blink command
    Wire.write(n); // count
    Wire.write(wait / 10); // time
    Wire.endTransmission();
}

void backspace(void) {
    debug.println("backspace i2c");
    
    Wire.beginTransmission(OWN_I2C_ADDRESS);
    Wire.write(0x02); // backspace command
    Wire.endTransmission();
}

void write_to_all(const char *a, const char *b,
                  const char *c, const char *d, int num_input) {
    const char *lines[4] = { a, b, c, d };
    
    //debug.println("write_to_all i2c");
    
    for (int i = 0; i < 4; i++) {
        Wire.beginTransmission(OWN_I2C_ADDRESS);
        Wire.write(0x03); // display command
    
        Wire.write(i);
        
        int l = strlen(lines[i]);
        Wire.write(l);
        
        for (int n = 0; n < l; n++) {
            Wire.write(lines[i][n]);
        }
    
        Wire.endTransmission();
    }
    
    Wire.beginTransmission(OWN_I2C_ADDRESS);
    Wire.write(0x04); // display command
    Wire.write((int8_t)num_input);
    Wire.endTransmission();
    
    write_lcd_to_serial(a, b, c, d);
    
#ifdef PLATFORM_ESP
    wifi_set_message_buffer(a, b, c, d);
    wifi_send_status_broadcast();
#endif // PLATFORM_ESP
}

#endif // ! FUNCTION_UI

#endif // FUNCTION_CONTROL
