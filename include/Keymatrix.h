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

#ifndef _KEYMATRIX_H_
#define _KEYMATRIX_H_

#ifdef FUNCTION_UI

#include <CircularBuffer.h>

class Keymatrix {
public:
    class Event {
    public:
        enum EventType {
            button_down,
            button_up,
            no_event
        };
        
        Event(EventType _type, int _row, int _col);
        EventType getType(void);
        int getRow(void);
        int getCol(void);
        
        // helper for 4x3 telephone keypad
        // -1 is *, -2 is #, or digits 0-9
        int getNum(void);
        
    private:
        EventType type;
        int row, col;
    };
    
    Keymatrix(int _rows, int _cols);
    ~Keymatrix(void);
    
    // first rows, then cols
    void setPins(int _pins[]);
    void setDebounce(unsigned long ms);
    
    void scan(void);
    
    bool hasEvent(void);
    Event getEvent(void);

private:
    unsigned long debounce;
    const static unsigned long default_debounce = 5;
    unsigned long last_scan_time;
    
    int rows, cols;
    int *pins;
    bool *lastPressed;
    bool *lastState;
    
    CircularBuffer<Event *, 32> events;
};

#endif // FUNCTION_UI

#endif // _KEYMATRIX_H_
