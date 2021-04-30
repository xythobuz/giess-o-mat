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

#include "Plants.h"
#include "DebugLog.h"
#include "Statemachine.h"
#include "config.h"

Statemachine::DigitBuffer::DigitBuffer(int _size) {
    size = _size;
    pos = 0;
    digits = new int[size];
}

Statemachine::DigitBuffer::~DigitBuffer() {
    delete digits;
}

bool Statemachine::DigitBuffer::spaceLeft(void) {
    return (pos < size);
}

bool Statemachine::DigitBuffer::hasDigits(void) {
    return (pos > 0);
}

int Statemachine::DigitBuffer::countDigits(void) {
    return pos;
}

void Statemachine::DigitBuffer::addDigit(int d) {
    if (spaceLeft()) {
        digits[pos] = d;
        pos++;
    }
}

void Statemachine::DigitBuffer::removeDigit(void) {
    if (hasDigits()) {
        pos--;
    }
}

void Statemachine::DigitBuffer::clear(void) {
    pos = 0;
}

uint32_t Statemachine::DigitBuffer::getNumber(void) {
    uint32_t fact = 1;
    uint32_t sum = 0;
    for (int i = (pos - 1); i >= 0; i--) {
        sum += digits[i] * fact;
        fact *= 10;
    }
    return sum;
}

static const char *state_names[] = {
    stringify(init),
    stringify(menu),
    stringify(auto_mode),
    stringify(auto_fert),
    stringify(auto_fert_run),
    stringify(auto_tank_run),
    stringify(auto_plant),
    stringify(auto_plant_run),
    stringify(auto_done),
    stringify(menu_pumps),
    stringify(menu_pumps_time),
    stringify(menu_pumps_go),
    stringify(menu_pumps_run),
    stringify(menu_pumps_done),
    stringify(menu_valves),
    stringify(menu_valves_time),
    stringify(menu_valves_go),
    stringify(menu_valves_run),
    stringify(menu_valves_done),
    stringify(error)
};

const char *Statemachine::getStateName(void) {
    return state_names[state];
}

Statemachine::Statemachine(print_fn _print, backspace_fn _backspace)
        : db(7) {
    state = init;
    old_state = init;
    print = _print;
    backspace = _backspace;
    
    selected_id = 0;
    selected_time = 0;
    start_time = 0;
    stop_time = 0;
    last_animation_time = 0;
    error_condition = "";
}

void Statemachine::begin(void) {
    switch_to(init);
}

void Statemachine::input(int n) {
    if (state == init) {
        switch_to(menu);
    } else if (state == menu) {
        if (n == 1) {
            switch_to(auto_mode);
        } else if (n == 2) {
            switch_to(menu_pumps);
        } else if (n == 3) {
            switch_to(menu_valves);
        } else if ((n == -1) || (n == -2)) {
            switch_to(init);
        }
    } else if (state == auto_mode) {
        if (n == 1) {
            switch_to(auto_fert);
        } else if (n == 2) {
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                plants.openWaterInlet();
                selected_id = plants.countPlants() + 1;
                selected_time = MAX_TANK_FILL_TIME;
                start_time = millis();
                switch_to(auto_tank_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(auto_mode);
            } else if (wl == Plants::invalid) {
                error_condition = "Invalid sensor state";
                state = auto_mode;
                switch_to(error);
            }
        } else if (n == 3) {
            switch_to(auto_plant);
        } else if ((n == -1) || (n == -2)) {
            switch_to(menu);
        }
    } else if (state == auto_fert) {
        if ((n >= 1) && (n <= 3)) {
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                plants.startFertilizer(n - 1);
                selected_id = n;
                selected_time = AUTO_PUMP_RUNTIME;
                start_time = millis();
                switch_to(auto_fert_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(auto_mode);
            } else if (wl == Plants::invalid) {
                error_condition = "Invalid sensor state";
                state = auto_mode;
                switch_to(error);
            }
        } else if ((n == -1) || (n == -2)) {
            switch_to(auto_mode);
        }
    } else if (state == auto_fert_run) {
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
    } else if (state == auto_tank_run) {
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
    } else if (state == auto_plant) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(auto_mode);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }
            
            selected_id = number_input();
            
            if ((selected_id <= 0) || (selected_id > plants.countPlants())) {
                error_condition = "Invalid plant ID!";
                switch_to(error);
            } else {
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                    plants.startPlant(selected_id - 1);
                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    switch_to(auto_plant_run);
                } else if (wl == Plants::empty) {
                    stop_time = millis();
                    switch_to(auto_mode);
                } else if (wl == Plants::invalid) {
                    error_condition = "Invalid sensor state";
                    state = auto_mode;
                    switch_to(error);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == auto_plant_run) {
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
    } else if (state == auto_done) {
        switch_to(auto_mode);
    } else if (state == menu_pumps) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(menu);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }
            
            selected_id = number_input();
            
            if ((selected_id <= 0) || (selected_id > plants.countFertilizers())) {
                error_condition = "Invalid pump ID!";
                switch_to(error);
            } else {
                switch_to(menu_pumps_time);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == menu_pumps_time) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(menu_pumps);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }
            
            selected_time = number_input();
            
            if ((selected_time <= 0) || (selected_time > 120)) {
                error_condition = "Invalid time range!";
                switch_to(error);
            } else {
                switch_to(menu_pumps_go);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == menu_pumps_go) {
        if (n == -2) {
            start_time = millis();
            last_animation_time = start_time;
            
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                plants.startFertilizer(selected_id - 1);
                switch_to(menu_pumps_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(menu_pumps_done);
            } else if (wl == Plants::invalid) {
                error_condition = "Invalid sensor state";
                state = menu_pumps;
                switch_to(error);
            }
        } else {
            switch_to(menu_pumps_time);
        }
    } else if (state == menu_pumps_run) {
            plants.abort();
            stop_time = millis();
            switch_to(menu_pumps_done);
    } else if (state == menu_pumps_done) {
        switch_to(menu);
    } else if (state == menu_valves) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(menu);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }
            
            selected_id = number_input();
            
            if ((selected_id <= 0) || (selected_id > (plants.countPlants() + 1))) {
                error_condition = "Invalid valve ID!";
                switch_to(error);
            } else {
                switch_to(menu_valves_time);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == menu_valves_time) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(menu_valves);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }
            
            selected_time = number_input();
            
            if ((selected_time <= 0) || (selected_time > 120)) {
                error_condition = "Invalid time range!";
                switch_to(error);
            } else {
                switch_to(menu_valves_go);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == menu_valves_go) {
        if (n == -2) {
            start_time = millis();
            last_animation_time = start_time;
            
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                if (selected_id >= (plants.countPlants() + 1)) {
                    plants.openWaterInlet();
                } else {
                    plants.startPlant(selected_id - 1);
                }
                
                switch_to(menu_valves_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(menu_valves_done);
            } else if (wl == Plants::invalid) {
                error_condition = "Invalid sensor state";
                state = menu_valves;
                switch_to(error);
            }
        } else {
            switch_to(menu_valves_time);
        }
    } else if (state == menu_valves_run) {
            plants.abort();
            stop_time = millis();
            switch_to(menu_valves_done);
    } else if (state == menu_valves_done) {
        switch_to(menu);
    } else if (state == error) {
        if (old_state != error) {
            switch_to(old_state);
        } else {
            switch_to(menu);
        }
    }
}

uint32_t Statemachine::number_input(void) {
    for (int i = 0; i < db.countDigits(); i++) {
        backspace();
    }
    
    uint32_t n = db.getNumber();
    db.clear();
    
    debug.print("Whole number input: ");
    debug.println(n);
    
    return n;
}

void Statemachine::act(void) {
    if ((state == menu_pumps_run) || (state == menu_valves_run)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            // stop if timeout has been reached
            plants.abort();
            stop_time = millis();
            switch_to((state == menu_pumps_run) ? menu_pumps_done : menu_valves_done);
        } else if ((millis() - last_animation_time) >= 500) {
            // update animation if needed
            last_animation_time = millis();
            switch_to(state);
        }
    }
    
#ifdef CHECK_SENSORS_VALVE_PUMP_MENU
    if ((state == menu_pumps_run) || ((state == menu_valves_run) && (selected_id == (plants.countPlants() + 1)))) {
        // check water level state
        auto wl = plants.getWaterlevel();
        if (wl == Plants::full) {
            plants.abort();
            stop_time = millis();
            switch_to((state == menu_pumps_run) ? menu_pumps_done : menu_valves_done);
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = "Invalid sensor state";
            state = (state == menu_pumps_run) ? menu_pumps : menu_valves;
            switch_to(error);
        }
    }
    
    if ((state == menu_valves_run) && (selected_id <= plants.countPlants())) {
        // check water level state
        auto wl = plants.getWaterlevel();
        if (wl == Plants::empty) {
            plants.abort();
            stop_time = millis();
            switch_to(menu_valves_done);
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = "Invalid sensor state";
            state = menu_valves;
            switch_to(error);
        }
    }
#endif
    
    if ((state == auto_fert_run) || (state == auto_tank_run)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            // stop if timeout has been reached
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
        } else if ((millis() - last_animation_time) >= 500) {
            // update animation if needed
            last_animation_time = millis();
            switch_to(state);
        }
        
        // check water level state
        auto wl = plants.getWaterlevel();
        if (wl == Plants::full) {
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = "Invalid sensor state";
            state = auto_mode;
            switch_to(error);
        }
    }
        
    if (state == auto_plant_run) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            // stop if timeout has been reached
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
        } else if ((millis() - last_animation_time) >= 500) {
            // update animation if needed
            last_animation_time = millis();
            switch_to(state);
        }
        
        // check water level state
        auto wl = plants.getWaterlevel();
        if (wl == Plants::empty) {
            plants.abort();
            stop_time = millis();
            switch_to(auto_done);
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = "Invalid sensor state";
            state = auto_mode;
            switch_to(error);
        }
    }
}

void Statemachine::switch_to(States s) {
    old_state = state;
    state = s;
    
    if (s == init) {
        String a = String("- Giess-o-mat V") + FIRMWARE_VERSION + String(" -");
        
        print(a.c_str(),
              "Usage:  Enter number",
              "* Delete prev. digit",
              "# Execute input num.",
              -1);
    } else if (s == menu) {
        print("------- Menu -------",
              "1: Automatic program",
              "2: Fertilizer pumps",
              "3: Outlet valves",
              -1);
    } else if (s == auto_mode) {
        print("------- Auto -------",
              "1: Add Fertilizer",
              "2: Fill Reservoir",
              "3: Water a plant",
              -1);
    } else if (s == auto_fert) {
        print("---- Fertilizer ----",
              "1: Vegetation Phase",
              "2: Bloom Phase",
              "3: Special",
              -1);
    } else if (s == auto_fert_run) {
        unsigned long runtime = millis() - start_time;
        String a = String("Runtime: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }
        
        print("---- Dispensing ----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if (s == auto_tank_run) {
        unsigned long runtime = millis() - start_time;
        String a = String("Runtime: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }
        
        print("--- Filling Tank ---",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if (s == auto_plant) {
        String a = String("(Input 1 to ") + String(plants.countPlants()) + String(")");
        
        print("--- Select Plant ---",
              "Which plant to water",
              a.c_str(),
              "Plant: ",
              3);
    } else if (s == auto_plant_run) {
        unsigned long runtime = millis() - start_time;
        String a = String("Runtime: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }
        
        print("----- Watering -----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if (s == auto_done) {
        String a = String("after ") + String((stop_time - start_time) / 1000UL) + String("s.");
        
        print("------- Done -------",
              "Dispensing finished",
              a.c_str(),
              "Hit any key for menu",
              -1);
    } else if (s == menu_pumps) {
        String a = String("(Input 1 to ") + String(plants.countFertilizers()) + String(")");
        
        print("------- Pump -------",
              "Please select pump",
              a.c_str(),
              "Pump: ",
              3);
    } else if (s == menu_pumps_time) {
        String header = String("------ Pump ") + String(selected_id) + String(" ------");
        
        print(header.c_str(),
              "Please set runtime",
              "(Input in seconds)",
              "Runtime: ",
              3);
    } else if (s == menu_pumps_go) {
        String a = String("Pump No. ") + String(selected_id);
        String b = String("Runtime ") + String(selected_time) + String('s');
        
        print("----- Confirm? -----",
              a.c_str(),
              b.c_str(),
              "           # Confirm",
              -1);
    } else if (s == menu_pumps_run) {
        unsigned long runtime = millis() - start_time;
        String a = String("Runtime: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }
        
        print("---- Dispensing ----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if (s == menu_pumps_done) {
        String a = String("after ") + String((stop_time - start_time) / 1000UL) + String("s.");
        
        print("------- Done -------",
              "Dispensing finished",
              a.c_str(),
              "Hit any key for menu",
              -1);
    } else if (s == menu_valves) {
        String a = String("(Input 1 to ") + String(plants.countPlants() + 1) + String(")");
        
        print("------ Valves ------",
              "Please select valve",
              a.c_str(),
              "Valve: ",
              3);
    } else if (s == menu_valves_time) {
        String header = String("----- Valve  ") + String(selected_id) + String(" -----");
        
        print(header.c_str(),
              "Please set runtime",
              "(Input in seconds)",
              "Runtime: ",
              3);
    } else if (s == menu_valves_go) {
        String a = String("Valve No. ") + String(selected_id);
        String b = String("Runtime ") + String(selected_time) + String('s');
        
        print("----- Confirm? -----",
              a.c_str(),
              b.c_str(),
              "           # Confirm",
              -1);
    } else if (s == menu_valves_run) {
        unsigned long runtime = millis() - start_time;
        String a = String("Runtime: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i <= anim; i++) {
            b += '#';
        }
        
        print("---- Dispensing ----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if (s == menu_valves_done) {
        String a = String("after ") + String((stop_time - start_time) / 1000UL) + String("s.");
        
        print("------- Done -------",
              "Dispensing finished",
              a.c_str(),
              "Hit any key for menu",
              -1);
    } else if (s == error) {
        print("------ Error! ------",
              "There is a problem:",
              error_condition.c_str(),
              "    Press any key...",
              -1);
    }
}
