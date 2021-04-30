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

// see https://github.com/sparkfun/SparkFun_SerLCD_Arduino_Library

#include <Arduino.h>
#include "SerialLCD.h"

#ifdef FUNCTION_UI

#define LCD_DELAY 3 // 3

SerialLCD::SerialLCD(int tx_pin) {
#if defined(PLATFORM_AVR)
    lcd = new SendOnlySoftwareSerial(tx_pin);
#elif defined(PLATFORM_ESP)
    lcd = new SoftwareSerial(tx_pin);
#endif
    
    lcd->begin(9600);
}

SerialLCD::~SerialLCD(void) {
    delete lcd;
}

void SerialLCD::init(void) {
    clear();
    cursor(0);
    setBacklight(255);
    
    lcd->write(0x7C);
    lcd->write(0x03); // 0x03=20 chars. 0x04=16 chars
    delay(LCD_DELAY);
    
    lcd->write(0x7C);
    lcd->write(0x05); // 0x05=4 lines, 0x06=2 lines
    delay(LCD_DELAY);
}

void SerialLCD::clear(void) {
    lcd->write(0xFE);
    lcd->write(0x01);
    delay(LCD_DELAY);
}

// 0 no cursor, 1 underline, 2 blinking, 3 both
void SerialLCD::cursor(int style) {
    lcd->write(0xFE);
    lcd->write(0x0C); // display on, no cursor
    delay(LCD_DELAY);
    
    if (style == 1) {
        lcd->write(0xFE);
        lcd->write(0x0E); // underline cursor on
        delay(LCD_DELAY);
    } else if (style == 2) {
        lcd->write(0xFE);
        lcd->write(0x0D); // blinking cursor on
        delay(LCD_DELAY);
    } else if (style == 3) {
        lcd->write(0xFE);
        lcd->write(0x0F); // both cursors on
        delay(LCD_DELAY);
    }
}

void SerialLCD::setBacklight(uint8_t val) {
    val = map(val, 0, 255, 0, 30);
    lcd->write(0x7C);
    lcd->write(0x80 + val);
    
    delay(LCD_DELAY);
}

void SerialLCD::position(int line, int col) {
    int cursor = 0;
    if (line ==  1) {
        cursor = 64;
    } else if (line == 2) {
        cursor = 20;
    } else if (line == 3) {
        cursor = 84;
    }
    
    lcd->write(0xFE);
    lcd->write(0x80 + cursor + col);
    delay(LCD_DELAY);
}

void SerialLCD::saveSplash(void) {
    lcd->write(0x7C);
    lcd->write(0x0A);
}

void SerialLCD::enableSplash(void) {
    lcd->write(0x7C);
    lcd->write(0x30);
}

void SerialLCD::disableSplash(void) {
    lcd->write(0x7C);
    lcd->write(0x31);
}
void SerialLCD::write(const char *text) {
    lcd->print(text);
    delay(LCD_DELAY);
}

void SerialLCD::write(int line, const char *text) {
    position(line, 0);
    write(text);
}

void SerialLCD::write(int line, int col, const char *text) {
    position(line, col);
    write(text);
}

#endif // FUNCTION_UI
