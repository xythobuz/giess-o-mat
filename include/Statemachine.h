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
#include "BoolField.h"

#define stringify( name ) # name

class Statemachine {
public:
    enum States {
        init = 0,
        door_select,
        menu_a, // manual, auto
        menu_b, // pumps, valves
        menu_c, // aux
        
        auto_mode_a, // select mode 1
        auto_mode_b, // select mode 2
        auto_mode_c, // select mode 3
        auto_fert_a, // select fertilizer 1
        auto_fert_b, // select fertilizer 2
        auto_fert_run,
        auto_tank_run,
        auto_stirr_run,
        auto_plant, // select plant
        auto_plant_kickstart_run,
        auto_plant_run,
        auto_done,

        fillnwater_plant, // select plants
        fillnwater_tank_run,
        fillnwater_kickstart_run,
        fillnwater_plant_run,

        fullauto_fert, // d i a
        fullauto_plant, // d i a
        fullauto_stirr_run, // d i a
        fullauto_fert_run, // d i a
        fullauto_tank_run, // d i a
        fullauto_kickstart_run, // d i a
        fullauto_plant_run, // d i a
        fullauto_plant_overrun, // d i a
        fullauto_tank_purge_run, // d i a
        fullauto_kickstart_purge_run, // d i a
        fullauto_plant_purge_run, // d i a
        fullauto_plant_purge_overrun, // d i a
        fullauto_done, // d i a

        automation_mode,
        
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
        
        menu_aux, // select aux channel
        menu_aux_time, // set runtime
        menu_aux_go, // running
        menu_aux_run,
        menu_aux_done,

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
    bool isIdle(void);
    
private:
    void switch_to(States s);
    uint32_t number_input(void);
    
    DigitBuffer db;
    States state, old_state;
    print_fn print;
    backspace_fn backspace;
    
    BoolField selected_plants;
    BoolField selected_ferts;
    uint32_t selected_id; // pump or valve id
    uint32_t selected_time; // runtime
    unsigned long start_time, stop_time, last_animation_time;
    String error_condition;
    unsigned long into_state_time;

    // used for calibrating, in fill'n'water mode
    bool filling_started_empty;
    bool watering_started_full;

    String menu_entered_digits;
};

#endif // _STATEMACHINE_H_
