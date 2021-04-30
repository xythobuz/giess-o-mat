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

#include "GPIOBank.h"

#ifdef PLATFORM_ESP
#include "WifiStuff.h"
#endif // PLATFORM_ESP

//#define GPIO_HIGH_AS_INPUT

GPIOBank::GPIOBank(int _size) {
    size = _size;
    pins = new int[size];
    out_state = new bool[size];
    is_output = false;
}

GPIOBank::~GPIOBank(void) {
    delete pins;
    delete out_state;
}

void GPIOBank::setPinNumbers(int _pins[]) {
    for (int i = 0; i < size; i++) {
        pins[i] = _pins[i];
    }
}

void GPIOBank::setOutput(void) {
    for (int i = 0; i < size; i++) {
#ifdef GPIO_HIGH_AS_INPUT
        pinMode(pins[i], INPUT);
#else
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], HIGH);
#endif
        out_state[i] = true;
    }
    is_output = true;
}

void GPIOBank::setInput(bool pullup) {
    for (int i = 0; i < size; i++) {
        if (pullup) {
            pinMode(pins[i], INPUT_PULLUP);
        } else {
            pinMode(pins[i], INPUT);
        }
    }
    is_output = false;
}

int GPIOBank::getSize(void) {
    return size;
}

void GPIOBank::setPin(int n, bool state) {
    if (!is_output) {
        return;
    }
    
    if ((n >= 0) && (n < size)) {
#ifdef GPIO_HIGH_AS_INPUT
        if (state) {
            pinMode(pins[n], OUTPUT);
            digitalWrite(pins[n], LOW);
        } else {
            pinMode(pins[n], INPUT);
        }
#else
        digitalWrite(pins[n], (!state) ? HIGH : LOW);
#endif

        out_state[n] = !state;
        
#ifdef PLATFORM_ESP
        wifi_schedule_websocket();
#endif // PLATFORM_ESP
    }
}

void GPIOBank::setAll(bool state) {
    for (int i = 0; i < size; i++) {
        setPin(i, state);
    }
}

bool GPIOBank::getPin(int n) {
    if ((n >= 0) && (n < size)) {
        if (is_output) {
            return !out_state[n];
        } else {
            return (!digitalRead(pins[n]));
        }
    } else {
        return false;
    }
}
