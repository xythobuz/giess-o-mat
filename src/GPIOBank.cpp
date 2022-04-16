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
#include "config.h"
#include "config_pins.h"

#ifdef TWI_GPIO
#include <PCF8574.h>
#include <Wire.h>
#endif

#ifdef PLATFORM_ESP
#include "WifiStuff.h"
#endif // PLATFORM_ESP

//#define GPIO_HIGH_AS_INPUT

// ----------------------------------------------------------------------------

#if defined(TWI_GPIO) && (I2C_GPIO_EXPANDER_COUNT > 0)
static uint8_t expand_addr[I2C_GPIO_EXPANDER_COUNT] = { I2C_GPIO_EXPANDER_ADDR };
static PCF8574 expand[I2C_GPIO_EXPANDER_COUNT];
#endif

void gpio_i2c_init(void) {
#if defined(TWI_GPIO) && (I2C_GPIO_EXPANDER_COUNT > 0)
    for (int i = 0; i < I2C_GPIO_EXPANDER_COUNT; i++) {
        expand[i].setAddress(expand_addr[i]);
        expand[i].begin(0xFF);
    }
#endif
}

static void gpio_pinMode(int pin, int value) {
    if (pin < 100) {
        pinMode(pin, value);
    } else if (pin < 0) {
        // ignore negative pin numbers
    } else {
#if defined(TWI_GPIO) && (I2C_GPIO_EXPANDER_COUNT > 0)
        pin -= 100;
        int ex = pin / 8;
        pin = pin % 8;
        if (ex < I2C_GPIO_EXPANDER_COUNT) {
            uint8_t mask = expand[ex].getButtonMask();
            if (value == OUTPUT) {
                mask &= ~(1 << pin);
            } else {
                mask |= (1 << pin);
            }
            expand[ex].setButtonMask(mask);
        }
#endif
    }
}

static void gpio_digitalWrite(int pin, int value) {
    if (pin < 100) {
        digitalWrite(pin, value);
    } else if (pin < 0) {
        // ignore negative pin numbers
    } else {
#if defined(TWI_GPIO) && (I2C_GPIO_EXPANDER_COUNT > 0)
        pin -= 100;
        int ex = pin / 8;
        pin = pin % 8;
        if (ex < I2C_GPIO_EXPANDER_COUNT) {
            expand[ex].write(pin, value);
        }
#endif
    }
}

static int gpio_digitalRead(int pin) {
    if (pin < 100) {
        return digitalRead(pin);
    } else if (pin < 0) {
        // ignore negative pin numbers
        return 0;
    } else {
#if defined(TWI_GPIO) && (I2C_GPIO_EXPANDER_COUNT > 0)
        pin -= 100;
        int ex = pin / 8;
        pin = pin % 8;
        if (ex < I2C_GPIO_EXPANDER_COUNT) {
            return expand[ex].readButton(pin);
        } else {
            return 0;
        }
#else
        return 0;
#endif
    }
}

// ----------------------------------------------------------------------------

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

int GPIOBank::getPinNumber(int pin) {
    if ((pin >= 0) && (pin < size)) {
        return pins[pin];
    } else {
        return -1;
    }
}

void GPIOBank::setOutput(void) {
    for (int i = 0; i < size; i++) {
#ifdef GPIO_HIGH_AS_INPUT
        gpio_pinMode(pins[i], INPUT);
#else
        gpio_pinMode(pins[i], OUTPUT);
        gpio_digitalWrite(pins[i], HIGH);
#endif
        out_state[i] = true;
    }
    is_output = true;
}

void GPIOBank::setInput(bool pullup) {
    for (int i = 0; i < size; i++) {
        if (pullup) {
            gpio_pinMode(pins[i], INPUT_PULLUP);
        } else {
            gpio_pinMode(pins[i], INPUT);
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
            gpio_pinMode(pins[n], OUTPUT);
            gpio_digitalWrite(pins[n], LOW);
        } else {
            gpio_pinMode(pins[n], INPUT);
        }
#else
        gpio_digitalWrite(pins[n], (!state) ? HIGH : LOW);
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
            return (!gpio_digitalRead(pins[n]));
        }
    } else {
        return false;
    }
}
