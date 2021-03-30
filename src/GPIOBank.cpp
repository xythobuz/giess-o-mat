#include <Arduino.h>
#include "GPIOBank.h"

GPIOBank::GPIOBank(int _size) {
    size = _size;
    pins = new int[size];
}

GPIOBank::~GPIOBank(void) {
    delete pins;
}

void GPIOBank::setPinNumbers(int _pins[]) {
    for (int i = 0; i < size; i++) {
        pins[i] = _pins[i];
    }
}

void GPIOBank::setOutput(void) {
    for (int i = 0; i < size; i++) {
        pinMode(pins[i], OUTPUT);
    }
}

void GPIOBank::setInput(bool pullup) {
    for (int i = 0; i < size; i++) {
        if (pullup) {
            pinMode(pins[i], INPUT_PULLUP);
        } else {
            pinMode(pins[i], INPUT);
        }
    }
}

int GPIOBank::getSize(void) {
    return size;
}

void GPIOBank::setPin(int n, bool state) {
    if ((n >= 0) && (n < size)) {
        digitalWrite(pins[n], state ? HIGH : LOW);
    }
}

void GPIOBank::setAll(bool state) {
    for (int i = 0; i < size; i++) {
        setPin(i, state);
    }
}

bool GPIOBank::getPin(int n) {
    if ((n >= 0) && (n < size)) {
        return digitalRead(pins[n]);
    } else {
        return LOW;
    }
}
