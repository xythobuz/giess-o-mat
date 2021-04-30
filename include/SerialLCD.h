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

#ifndef _SERIAL_LCD_H_
#define _SERIAL_LCD_H_

#ifdef FUNCTION_UI

#if defined(PLATFORM_AVR)
#include <SendOnlySoftwareSerial.h>
#elif defined(PLATFORM_ESP)
#include <SoftwareSerial.h>
#else
#error platform not supported
#endif

class SerialLCD {
public:
    SerialLCD(int tx_pin);
    ~SerialLCD(void);
    
    void init(void);
    void clear(void);
    void setBacklight(uint8_t val);
    
    void saveSplash(void);
    void enableSplash(void);
    void disableSplash(void);
    
    // 0 no cursor, 1 underline, 2 blinking, 3 both
    void cursor(int style);
    
    void position(int line, int col);
    
    void write(const char *text);
    void write(int line, const char *text);
    void write(int line, int col, const char *text);
    
private:
#if defined(PLATFORM_AVR)
    SendOnlySoftwareSerial *lcd;
#elif defined(PLATFORM_ESP)
    SoftwareSerial *lcd;
#endif
};

#endif // FUNCTION_UI

#endif // _SERIAL_LCD_H_
