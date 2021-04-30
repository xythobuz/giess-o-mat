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

#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include <Arduino.h>

#define stringify( name ) # name

class Statemachine {
public:
    enum States {
        init = 0,
        menu, // auto, pumps, valves
        
        auto_mode, // select mode
        auto_fert, // select fertilizer
        auto_fert_run,
        auto_tank_run,
        auto_plant, // select plant
        auto_plant_run,
        auto_done,
        
        menu_pumps, // selet pump
        menu_pumps_time, // set runtime
        menu_pumps_go, // running
        menu_pumps_run,
        menu_pumps_done,
        
        menu_valves, // select valve
        menu_valves_time, // set runtime
        menu_valves_go, // running
        menu_valves_run,
        menu_valves_done,
        
        error
    };
    
    class DigitBuffer {
    public:
        DigitBuffer(int _size);
        ~DigitBuffer();
        
        bool spaceLeft(void);
        bool hasDigits(void);
        int countDigits(void);
        
        void addDigit(int d);
        void removeDigit(void);
        void clear(void);
        
        uint32_t getNumber(void);
        
    private:
        int size;
        int pos;
        int *digits;
    };
    
    typedef void (*print_fn)(const char *, const char *, const char *, const char *, int);
    
    typedef void (*backspace_fn)(void);
    
    Statemachine(print_fn _print, backspace_fn _backspace);
    void begin(void);
    
    void input(int n);
    void act(void);
    
    const char *getStateName(void);
    
private:
    void switch_to(States s);
    uint32_t number_input(void);
    
    DigitBuffer db;
    States state, old_state;
    print_fn print;
    backspace_fn backspace;
    
    uint32_t selected_id; // pump or valve id
    uint32_t selected_time; // runtime
    unsigned long start_time, stop_time, last_animation_time;
    String error_condition;
};

#endif // _STATEMACHINE_H_
