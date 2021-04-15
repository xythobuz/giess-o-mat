#include <Arduino.h>
#include "Keymatrix.h"

#ifdef FUNCTION_UI

//#define DEBUG_PRINT_MATRIX_STATE

#ifdef DEBUG_PRINT_MATRIX_STATE
#include "DebugLog.h"
#endif // DEBUG_PRINT_MATRIX_STATE

Keymatrix::Event::Event(EventType _type, int _row, int _col) {
    type = _type;
    row = _row;
    col = _col;
}

Keymatrix::Event::EventType Keymatrix::Event::getType(void) {
    return type;
}

int Keymatrix::Event::getRow(void) {
    return row;
}

int Keymatrix::Event::getCol(void) {
    return col;
}

// -1 is *, -2 is #, or digits 0-9
int Keymatrix::Event::getNum(void) {
    if (row == 3) {
        switch (col) {
        case 0:
            return -1; // *
            
        case 2:
            return -2; // #
            
        default:
            return 0;
        }
    } else {
        return (row * 3) + col + 1;
    }
}

Keymatrix::Keymatrix(int _rows, int _cols) {
    debounce = default_debounce;
    last_scan_time = 0;
    
    rows = _rows;
    cols = _cols;
    
    pins = new int[rows + cols];
    lastPressed = new bool[rows * cols];
    lastState = new bool[rows * cols];
    
    for (int i = 0; i < (rows * cols); i++) {
        lastPressed[i] = false;
        lastState[i] = false;
    }
}

Keymatrix::~Keymatrix(void) {
    delete pins;
    delete lastPressed;
    delete lastState;
}

// first rows, then cols
void Keymatrix::setPins(int _pins[]) {
    for (int i = 0; i < (rows + cols); i++) {
        pins[i] = _pins[i];
        
        // rows as outputs, cols as inputs
        if (i < rows) {
            //pinMode(pins[i], OUTPUT);
            pinMode(pins[i], INPUT);
        } else {
            pinMode(pins[i], INPUT_PULLUP);
        }
    }
}

void Keymatrix::setDebounce(unsigned long ms) {
    debounce = ms;
}

void Keymatrix::scan(void) {
    // only continue when enough time has passed
    unsigned long current_time = millis();
    if (current_time < (last_scan_time + debounce)) {
        return;
    }
    last_scan_time = current_time;
    
    // disable all rows
    for (int r = 0; r < rows; r++) {
        //digitalWrite(pins[r], HIGH);
        pinMode(pins[r], INPUT);
    }
    
    int buttons = rows * cols;
    bool pressed[buttons];
    
    // go through all rows
    for (int r = 0; r < rows; r++) {
        // enable current row
        pinMode(pins[r], OUTPUT);
        digitalWrite(pins[r], LOW);
        
        // read out all columns
        for (int c = 0; c < cols; c++) {
            int v = digitalRead(pins[rows + c]);
            pressed[(r * cols) + c] = (v == LOW);
        }
        
        // disable current row
        //digitalWrite(pins[r], HIGH);
        pinMode(pins[r], INPUT);
    }
    
#ifdef DEBUG_PRINT_MATRIX_STATE
    for (int i = 0; i < buttons; i++) {
        debug.print(pressed[i] ? "1" : "0");
        if (i < (buttons - 1)) {
            debug.print(" ");
        } else {
            debug.println();
        }
    }
#endif // DEBUG_PRINT_MATRIX_STATE
    
    for (int i = 0; i < buttons; i++) {
        // debounce - compare to previous state
        if ((lastPressed[i] == pressed[i]) && (pressed[i] != lastState[i])) {
            lastState[i] = pressed[i];
            int c = i % cols;
            int r = i / cols;
            Event::EventType et = (pressed[i]) ? Event::button_down : Event::button_up;
            events.push(new Event(et, r, c));
        }
        
        // save current state for next time
        lastPressed[i] = pressed[i];
    }
}

bool Keymatrix::hasEvent(void) {
    return !events.isEmpty();
}

Keymatrix::Event Keymatrix::getEvent(void) {
    if (hasEvent()) {
        Event *e = events.shift();
        Event e_copy = *e;
        delete e;
        return e_copy;
    } else {
        return Keymatrix::Event(Event::no_event, -1, -1);
    }
}

#endif // FUNCTION_UI
