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
#include "WifiStuff.h"
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
    stringify(menu_a),
    stringify(menu_b),
    stringify(auto_mode_a),
    stringify(auto_mode_b),
    stringify(auto_fert),
    stringify(auto_fert_run),
    stringify(auto_tank_run),
    stringify(auto_plant),
    stringify(auto_plant_run),
    stringify(auto_done),
    stringify(fillnwater_plant),
    stringify(fillnwater_tank_run),
    stringify(fillnwater_plant_run),
    stringify(automation_mode),
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

bool Statemachine::isIdle(void) {
    return state == init;
}

Statemachine::Statemachine(print_fn _print, backspace_fn _backspace)
        : db(7), selected_plants(plants.countPlants()) {
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
    into_state_time = 0;
}

void Statemachine::begin(void) {
    switch_to(init);
}

void Statemachine::input(int n) {
    if (state == init) {
        switch_to(menu_a);
    } else if ((state == menu_a) || (state == menu_b)) {
        if (n == 1) {
            switch_to(auto_mode_a);
        } else if (n == 2) {
            switch_to(automation_mode);
        } else if (n == 3) {
            switch_to(menu_pumps);
        } else if (n == 4) {
            switch_to(menu_valves);
        } else if (n == -1) {
            switch_to(init);
        } else if (n == -2) {
            switch_to((state == menu_a) ? menu_b : menu_a);
        }
    } else if (state == automation_mode) {
        // TODO
        switch_to(menu_a);
    } else if ((state == auto_mode_a) || (state == auto_mode_b)) {
        if (n == 1) {
            switch_to(auto_fert);
        } else if (n == 2) {
            selected_plants.clear();
            switch_to(fillnwater_plant);
        } else if (n == 3) {
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                plants.openWaterInlet();
                selected_id = plants.countPlants() + 1;
                selected_time = MAX_TANK_FILL_TIME;
                start_time = millis();
                switch_to(auto_tank_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(auto_mode_a);
            } else if (wl == Plants::invalid) {
                error_condition = "Invalid sensor state";
                state = auto_mode_a;
                switch_to(error);
            }
        } else if (n == 4) {
            selected_plants.clear();
            switch_to(auto_plant);
        } else if (n == -1) {
            switch_to(menu_a);
        } else if (n == -2) {
            switch_to((state == auto_mode_a) ? auto_mode_b : auto_mode_a);
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
                switch_to(auto_mode_a);
            } else if (wl == Plants::invalid) {
                error_condition = "Invalid sensor state";
                state = auto_mode_a;
                switch_to(error);
            }
        } else if ((n == -1) || (n == -2)) {
            switch_to(auto_mode_a);
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
                switch_to(auto_mode_b);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i);
                        }
                    }
                    
                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    switch_to(auto_plant_run);
                } else if (wl == Plants::empty) {
                    stop_time = millis();
                    switch_to(auto_mode_b);
                } else if (wl == Plants::invalid) {
                    error_condition = "Invalid sensor state";
                    state = auto_mode_b;
                    switch_to(error);
                }
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > plants.countPlants())) {
                    error_condition = "Invalid plant ID!";
                    switch_to(error);
                } else {
                    selected_plants.set(selected_id - 1);
                    switch_to(auto_plant);
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
        switch_to(auto_mode_a);
    } else if (state == menu_pumps) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(menu_b);
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
    } else if (state == fillnwater_plant) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(auto_mode_a);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                int found = 0;
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        found = 1;
                        break;
                    }
                }
                if (found != 0) {
                    auto wl = plants.getWaterlevel();
                    if ((wl != Plants::full) && (wl != Plants::invalid)) {
                        plants.openWaterInlet();
                        selected_id = plants.countPlants() + 1;
                        selected_time = MAX_TANK_FILL_TIME;
                        start_time = millis();
                        switch_to(fillnwater_tank_run);
                    } else if (wl == Plants::full) {
                        stop_time = millis();
                        auto wl = plants.getWaterlevel();
                        if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                            for (int i = 0; i < plants.countPlants(); i++) {
                                if (selected_plants.isSet(i)) {
                                    plants.startPlant(i);
                                }
                            }

                            selected_time = MAX_AUTO_PLANT_RUNTIME;
                            start_time = millis();
                            switch_to(fillnwater_plant_run);
                        } else if (wl == Plants::empty) {
                            stop_time = millis();
                            switch_to(auto_mode_a);
                        } else if (wl == Plants::invalid) {
                            error_condition = "Invalid sensor state";
                            state = auto_mode_a;
                            switch_to(error);
                        }
                    } else if (wl == Plants::invalid) {
                        error_condition = "Invalid sensor state";
                        state = auto_mode_a;
                        switch_to(error);
                    }
                }
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > plants.countPlants())) {
                    error_condition = "Invalid plant ID!";
                    switch_to(error);
                } else {
                    selected_plants.set(selected_id - 1);
                    switch_to(fillnwater_plant);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == fillnwater_tank_run) {
        plants.abort();
        stop_time = millis();
        auto wl = plants.getWaterlevel();
        if ((wl != Plants::empty) && (wl != Plants::invalid)) {
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    plants.startPlant(i);
                }
            }

            selected_time = MAX_AUTO_PLANT_RUNTIME;
            start_time = millis();
            switch_to(fillnwater_plant_run);
        } else if (wl == Plants::empty) {
            switch_to(auto_mode_a);
        } else if (wl == Plants::invalid) {
            error_condition = "Invalid sensor state";
            state = auto_mode_a;
            switch_to(error);
        }
    } else if (state == fillnwater_plant_run) {
        plants.abort();
        stop_time = millis();
        switch_to(auto_done);
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
            
            if ((selected_time <= 0) || (selected_time > MAX_PUMP_RUNTIME)) {
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
        switch_to(menu_b);
    } else if (state == menu_valves) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
            } else {
                switch_to(menu_b);
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
            
            if ((selected_time <= 0) || (selected_time > MAX_VALVE_RUNTIME)) {
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
        switch_to(menu_b);
    } else if (state == error) {
        if (old_state != error) {
            switch_to(old_state);
        } else {
            switch_to(menu_a);
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
    
#ifdef CHECK_SENSORS_VALVE_PUMP_MENU_FULL
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
#endif // CHECK_SENSORS_VALVE_PUMP_MENU_FULL
    
#ifdef CHECK_SENSORS_VALVE_PUMP_MENU_EMPTY
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
#endif // CHECK_SENSORS_VALVE_PUMP_MENU_EMPTY
    
    if ((state == auto_fert_run) || (state == auto_tank_run) || (state == fillnwater_tank_run)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            // stop if timeout has been reached
            plants.abort();
            stop_time = millis();
            if (state == fillnwater_tank_run) {
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i);
                        }
                    }

                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    switch_to(fillnwater_plant_run);
                } else if (wl == Plants::empty) {
                    stop_time = millis();
                    switch_to(auto_done);
                } else if (wl == Plants::invalid) {
                    error_condition = "Invalid sensor state";
                    state = auto_mode_a;
                    switch_to(error);
                }
            } else {
                switch_to(auto_done);
            }
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
            if (state == fillnwater_tank_run) {
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i);
                        }
                    }

                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    switch_to(fillnwater_plant_run);
                } else if (wl == Plants::empty) {
                    stop_time = millis();
                    switch_to(auto_mode_a);
                } else if (wl == Plants::invalid) {
                    error_condition = "Invalid sensor state";
                    state = auto_mode_a;
                    switch_to(error);
                }
            } else {
                switch_to(auto_done);
            }
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = "Invalid sensor state";
            state = auto_mode_a;
            switch_to(error);
        }
    }

    if ((state == auto_plant_run) || (state == fillnwater_plant_run)) {
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
            state = auto_mode_a;
            switch_to(error);
        }
    }

    if ((state == menu_a) || (state == menu_b) || (state == automation_mode)
            || (state == auto_mode_a) || (state == auto_mode_b)
            || (state == auto_fert) || (state == auto_done)
            || (state == auto_plant) || (state == fillnwater_plant)
            || (state == menu_pumps) || (state == menu_pumps_time)
            || (state == menu_pumps_go) || (state == menu_pumps_done)
            || (state == menu_valves) || (state == menu_valves_time)
            || (state == menu_valves_go) || (state == menu_valves_done)) {
        unsigned long runtime = millis() - into_state_time;
        if (runtime >= BACK_TO_IDLE_TIMEOUT) {
            debug.print("Idle timeout reached in state ");
            debug.println(state_names[state]);
            switch_to(init);
        }
    }
}

void Statemachine::switch_to(States s) {
    old_state = state;
    state = s;
    into_state_time = millis();

    if (old_state != state) {
        // don't spam log with every animation state "change"
        debug.print("switch_to ");
        debug.print(state_names[old_state]);
        debug.print(" --> ");
        debug.println(state_names[state]);
    }
    
    if (s == init) {
        String a = String("- Giess-o-mat V") + FIRMWARE_VERSION + String(" -");
        
        print(a.c_str(),
              "Usage:  Enter number",
              "* Delete prev. digit",
              "# Execute input num.",
              -1);
    } else if (s == menu_a) {
        print("----- Menu 1/2 -----",
              "1: Manual Operation",
              "2: Automation",
              "#: Go to page 2/2...",
              -1);
    } else if (s == menu_b) {
        print("----- Menu 2/2 -----",
              "3: Fertilizer pumps",
              "4: Outlet valves",
              "#: Go to page 1/2...",
              -1);
    } else if (state == automation_mode) {
        // TODO
        print("---- Automation ----",
              "TODO NOT IMPLEMENTED",
              "TODO NOT IMPLEMENTED",
              "TODO NOT IMPLEMENTED",
              -1);
    } else if (s == auto_mode_a) {
        print("---- Manual 1/2 ----",
              "1: Add Fertilizer",
              "2: Fill 'n' Water",
              "#: Go to page 2/2...",
              -1);
    } else if (s == auto_mode_b) {
        print("---- Manual 2/2 ----",
              "3: Fill Reservoir",
              "4: Water a plant",
              "#: Go to page 1/2...",
              -1);
    } else if (s == auto_fert) {
        print("---- Fertilizer ----",
              "1: Vegetation Phase",
              "2: Bloom Phase",
              "3: Special",
              -1);
    } else if (s == auto_fert_run) {
        unsigned long runtime = millis() - start_time;
        String a = String("Time: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
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
    } else if ((s == auto_tank_run) || (s == fillnwater_tank_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String("Time: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
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
    } else if ((s == auto_plant) || (s == fillnwater_plant)) {
        String a = String("(Input 1 to ") + String(plants.countPlants()) + String(")");
        
        print("--- Select Plant ---",
              "Leave empty if done!",
              a.c_str(),
              "Plant: ",
              3);
    } else if ((s == auto_plant_run) || (s == fillnwater_plant_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String("Time: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
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

#if defined(PLATFORM_ESP)
        unsigned long runtime = stop_time - start_time;
        if ((old_state == auto_plant_run) || (old_state == fillnwater_plant_run)) {
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    bool success = wifi_write_database(runtime / 1000, "plant", i + 1);
                    if (!success) {
                        debug.print("Error writing to InfluxDB ");
                        debug.print(INFLUXDB_HOST);
                        debug.print(":");
                        debug.print(INFLUXDB_PORT);
                        debug.print("/");
                        debug.print(INFLUXDB_DATABASE);
                        debug.println("/plant");
                    }
                }
            }
        } else if (old_state == auto_fert_run) {
            bool success = wifi_write_database(runtime / 1000, "fertilizer", selected_id);
            if (!success) {
                debug.print("Error writing to InfluxDB ");
                debug.print(INFLUXDB_HOST);
                debug.print(":");
                debug.print(INFLUXDB_PORT);
                debug.print("/");
                debug.print(INFLUXDB_DATABASE);
                debug.println("/fertilizer");
            }
        }
#endif // PLATFORM_ESP
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
        String a = String("Time: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
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

#if defined(PLATFORM_ESP)
        unsigned long runtime = stop_time - start_time;
        bool success = wifi_write_database(runtime / 1000, "fertilizer", selected_id);
        if (!success) {
            debug.print("Error writing to InfluxDB ");
            debug.print(INFLUXDB_HOST);
            debug.print(":");
            debug.print(INFLUXDB_PORT);
            debug.print("/");
            debug.print(INFLUXDB_DATABASE);
            debug.println("/fertilizer");
        }
#endif // PLATFORM_ESP
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
        String a = String("Time: ") + String(runtime / 1000UL) + String("s / ") + String(selected_time) + String('s');
        
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

#if defined(PLATFORM_ESP)
        unsigned long runtime = stop_time - start_time;
        if (selected_id <= plants.countPlants()) {
            bool success = wifi_write_database(runtime / 1000, "plant", selected_id);
            if (!success) {
                debug.print("Error writing to InfluxDB ");
                debug.print(INFLUXDB_HOST);
                debug.print(":");
                debug.print(INFLUXDB_PORT);
                debug.print("/");
                debug.print(INFLUXDB_DATABASE);
                debug.println("/plant");
            }
        }
#endif // PLATFORM_ESP
    } else if (s == error) {
        print("------ Error! ------",
              "There is a problem:",
              error_condition.c_str(),
              "    Press any key...",
              -1);
    } else {
        debug.print("Invalid state ");
        debug.println(s);
    }
}
