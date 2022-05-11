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
#include "config_pins.h"

#ifdef FUNCTION_CONTROL

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
    stringify(door_select),
    stringify(menu_a),
    stringify(menu_b),
    stringify(menu_c),

    stringify(auto_mode_a),
    stringify(auto_mode_b),
    stringify(auto_mode_c),
    stringify(auto_fert_a),
    stringify(auto_fert_b),
    stringify(auto_fert_run),
    stringify(auto_tank_run),
    stringify(auto_stirr_run),
    stringify(auto_plant),
    stringify(auto_plant_kickstart_run),
    stringify(auto_plant_run),
    stringify(auto_done),

    stringify(fillnwater_plant),
    stringify(fillnwater_tank_run),
    stringify(fillnwater_kickstart_run),
    stringify(fillnwater_plant_run),

    stringify(fullauto_fert),
    stringify(fullauto_plant),
    stringify(fullauto_stirr_run),
    stringify(fullauto_fert_run),
    stringify(fullauto_tank_run),
    stringify(fullauto_kickstart_run),
    stringify(fullauto_plant_run),
    stringify(fullauto_plant_overrun),
    stringify(fullauto_tank_purge_run),
    stringify(fullauto_kickstart_purge_run),
    stringify(fullauto_plant_purge_run),
    stringify(fullauto_plant_purge_overrun),
    stringify(fullauto_done),

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

    stringify(menu_aux),
    stringify(menu_aux_time),
    stringify(menu_aux_go),
    stringify(menu_aux_run),
    stringify(menu_aux_done),

    stringify(error)
};

static int auto_pump_runtime[PUMP_COUNT] = AUTO_PUMP_RUNTIME;

const char *Statemachine::getStateName(void) {
    return state_names[state];
}

bool Statemachine::isIdle(void) {
    return state == init;
}

Statemachine::Statemachine(print_fn _print, backspace_fn _backspace)
        : db(7), selected_plants(plants.countPlants()),
        selected_ferts(plants.countFertilizers()) {
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
    filling_started_empty = false;
    watering_started_full = false;
    menu_entered_digits = "";
}

void Statemachine::begin(void) {
    switch_to(init);
}

void Statemachine::input(int n) {
    if (state == init) {
#if (LOCK_COUNT > 0) && defined(DOOR_LOCK_PIN)
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_a);
            }
        } else if (n == -2) {
            switch_to(menu_a);
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                //menu_entered_digits += String(n);
                menu_entered_digits += String("*");
                switch_to(state);
            } else {
                backspace();
            }

            uint32_t n = db.getNumber();
            if (n == DOOR_LOCK_PIN) {
                db.clear();
                menu_entered_digits = "";
                selected_plants.clear();
                switch_to(door_select);
            } else if (db.countDigits() >= DOOR_LOCK_PIN_MAX_DIGITS) {
                db.clear();
                menu_entered_digits = "";
                switch_to(state);
            }
        }
#else
        switch_to(menu_a);
#endif
    } else if (state == door_select) {
#if (LOCK_COUNT > 0)
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(init);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                for (int i = 0; i < LOCK_COUNT; i++) {
                    if (selected_plants.isSet(i)) {
                        plants.startAux(STIRRER_COUNT + i);
                        delay(DOOR_LOCK_ON_TIME);
                        plants.stopAux(STIRRER_COUNT + i);
                        delay(DOOR_LOCK_NEXT_DELAY);
                    }
                }
                plants.stopAllAux();
                switch_to(menu_a);
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > LOCK_COUNT)) {
                    error_condition = F("Invalid lock ID!");
                    switch_to(error);
                } else {
                    selected_plants.set(selected_id - 1);
                    menu_entered_digits = "";
                    switch_to(state);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
#else
        // should never be reached
        switch_to(menu_a);
#endif
    } else if ((state == menu_a) || (state == menu_b) || (state == menu_c)) {
        if (n == 1) {
            switch_to(auto_mode_a);
        } else if (n == 2) {
            switch_to(automation_mode);
        } else if (n == 3) {
            switch_to(menu_pumps);
        } else if (n == 4) {
            switch_to(menu_valves);
        } else if (n == 5) {
            switch_to(menu_aux);
#if (LOCK_COUNT > 0) && !defined(DOOR_LOCK_PIN)
        } else if (n == 6) {
            switch_to(door_select);
#endif
        } else if (n == -1) {
            switch_to(init);
        } else if (n == -2) {
            switch_to((state == menu_a) ? menu_b : ((state == menu_b) ? menu_c : menu_a));
        }
    } else if (state == automation_mode) {
        // TODO
        switch_to(menu_a);
    } else if ((state == auto_mode_a) || (state == auto_mode_b) || (state == auto_mode_c)) {
        if (n == 1) {
            selected_ferts.clear();
            switch_to(fullauto_fert);
        } else if (n == 2) {
            switch_to(auto_fert_a);
        } else if (n == 3) {
            selected_plants.clear();
            switch_to(fillnwater_plant);
        } else if (n == 4) {
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
                error_condition = F("Invalid sensor state");
                state = auto_mode_a;
                switch_to(error);
            }
        } else if (n == 5) {
            selected_plants.clear();
            switch_to(auto_plant);
        } else if (n == -1) {
            switch_to(menu_a);
        } else if (n == -2) {
            switch_to((state == auto_mode_a) ? auto_mode_b : ((state == auto_mode_b) ? auto_mode_c : auto_mode_a));
        }
    } else if ((state == auto_fert_a) || (state == auto_fert_b)) {
        // TODO fertilizer number currently "hardcoded" to 3 in UI
        if ((n >= 1) && (n <= 3)) {
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                plants.startFertilizer(n - 1);
                selected_id = n;
                selected_time = auto_pump_runtime[n - 1];
                start_time = millis();
                switch_to(auto_fert_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(auto_mode_a);
            } else if (wl == Plants::invalid) {
                error_condition = F("Invalid sensor state");
                state = auto_mode_a;
                switch_to(error);
            }
        } else if (n == 4) {
            for (int i = 0; i < STIRRER_COUNT; i++) {
                plants.startAux(i);
            }
            selected_id = 1;
            selected_time = AUTO_STIRR_RUNTIME;
            start_time = millis();
            switch_to(auto_stirr_run);
        } else if (n == -1) {
            switch_to(auto_mode_a);
        } else if (n == -2) {
            switch_to((state == auto_fert_a) ? auto_fert_b : auto_fert_a);
        }
    } else if (state == fullauto_stirr_run) {
        // allow user to stop stirring and continue with fertilizers
        plants.abort();
        stop_time = millis();
        if (selected_ferts.countSet() > 0) {
            selected_time = auto_pump_runtime[0];
            for (int i = 0; i < plants.countFertilizers(); i++) {
                if (auto_pump_runtime[i] > selected_time) {
                    selected_time = auto_pump_runtime[i];
                }

                if (selected_ferts.isSet(i)) {
                    plants.startFertilizer(i);
                }
            }

            start_time = millis();
            switch_to(fullauto_fert_run);
        } else {
            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                // if the waterlevel is currently empty, we
                // set a flag to record the time to fill
                filling_started_empty = (wl == Plants::empty);

                plants.openWaterInlet();
                selected_id = plants.countPlants() + 1;
                selected_time = MAX_TANK_FILL_TIME;
                start_time = millis();
                switch_to(fullauto_tank_run);
            } else if (wl == Plants::full) {
                // check if kickstart is required for this
                bool need_kickstart = false;
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        if (plants.getKickstart()->getPinNumber(i) >= 0) {
                            need_kickstart = true;
                        }
                    }
                }

                // start kickstart/valve as needed
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        plants.startPlant(i, need_kickstart);
                    }
                }

                watering_started_full = (wl == Plants::full);

                selected_time = MAX_AUTO_PLANT_RUNTIME;
                start_time = millis();
                if (need_kickstart) {
                    switch_to(fullauto_kickstart_run);
                } else {
                    switch_to(fullauto_plant_run);
                }
            } else if (wl == Plants::invalid) {
                plants.abort();
                error_condition = F("Invalid sensor state");
                state = fullauto_fert;
                switch_to(error);
            }
        }
    } else if (state == fullauto_fert_run) {
        // allow user to stop fertizilers and continue with tank filling
        auto wl = plants.getWaterlevel();
        if ((wl != Plants::full) && (wl != Plants::invalid)) {
            // if the waterlevel is currently empty, we
            // set a flag to record the time to fill
            filling_started_empty = (wl == Plants::empty);

            plants.openWaterInlet();
            selected_id = plants.countPlants() + 1;
            selected_time = MAX_TANK_FILL_TIME;
            start_time = millis();
            switch_to(fullauto_tank_run);
        } else if (wl == Plants::full) {
            // check if kickstart is required for this
            bool need_kickstart = false;
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    if (plants.getKickstart()->getPinNumber(i) >= 0) {
                        need_kickstart = true;
                    }
                }
            }

            // start kickstart/valve as needed
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    plants.startPlant(i, need_kickstart);
                }
            }

            watering_started_full = (wl == Plants::full);

            selected_time = MAX_AUTO_PLANT_RUNTIME;
            start_time = millis();
            if (need_kickstart) {
                switch_to(fullauto_kickstart_run);
            } else {
                switch_to(fullauto_plant_run);
            }
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = F("Invalid sensor state");
            state = fullauto_fert;
            switch_to(error);
        }
    } else if ((state == fullauto_tank_run)
            || (state == fullauto_tank_purge_run)) {
        // allow user to stop tank and continue with kickstart
        plants.abort();
        stop_time = millis();
        auto wl = plants.getWaterlevel();
        if ((wl != Plants::empty) && (wl != Plants::invalid)) {
            // check if kickstart is required for this
            bool need_kickstart = false;
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    if (plants.getKickstart()->getPinNumber(i) >= 0) {
                        need_kickstart = true;
                    }
                }
            }

            // start kickstart/valve as needed
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    plants.startPlant(i, need_kickstart);
                }
            }

            watering_started_full = (wl == Plants::full);

            selected_time = MAX_AUTO_PLANT_RUNTIME;
            start_time = millis();
            if (need_kickstart) {
                if (state == fullauto_tank_run) {
                    switch_to(fullauto_kickstart_run);
                } else {
                    switch_to(fullauto_kickstart_purge_run);
                }
            } else {
                if (state == fullauto_tank_run) {
                    switch_to(fullauto_plant_run);
                } else {
                    switch_to(fullauto_plant_purge_run);
                }
            }
        } else if (wl == Plants::empty) {
            stop_time = millis();
            switch_to(auto_done);
        } else if (wl == Plants::invalid) {
            error_condition = F("Invalid sensor state");
            state = auto_mode_a;
            switch_to(error);
        }
    } else if ((state == fullauto_kickstart_run)
            || (state == fullauto_kickstart_purge_run)) {
        // allow user to stop kickstarting and continue with plants
        plants.abort();
        selected_time = MAX_AUTO_PLANT_RUNTIME;
        start_time = millis();

        // start required valves
        for (int i = 0; i < plants.countPlants(); i++) {
            if (selected_plants.isSet(i)) {
                plants.startPlant(i, false);
            }
        }

        if (state == fullauto_kickstart_run) {
            switch_to(fullauto_plant_run);
        } else {
            switch_to(fullauto_plant_purge_run);
        }
    } else if ((state == auto_fert_run) || (state == auto_tank_run)
            || (state == auto_stirr_run) || (state == auto_plant_run)
            || (state == auto_plant_kickstart_run)
            || (state == fillnwater_kickstart_run)
            || (state == fullauto_plant_run)
            || (state == fullauto_plant_overrun)
            || (state == fullauto_plant_purge_run)
            || (state == fullauto_plant_purge_overrun)) {
        plants.abort();
        stop_time = millis();
        switch_to(auto_done);
    } else if (state == auto_plant) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(auto_mode_b);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                    // check if kickstart is required for this
                    bool need_kickstart = false;
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            if (plants.getKickstart()->getPinNumber(i) >= 0) {
                                need_kickstart = true;
                            }
                        }
                    }

                    // start kickstart/valve as needed
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i, need_kickstart);
                        }
                    }

                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    if (need_kickstart) {
                        switch_to(auto_plant_kickstart_run);
                    } else {
                        switch_to(auto_plant_run);
                    }
                } else if (wl == Plants::empty) {
                    stop_time = millis();
                    switch_to(auto_mode_b);
                } else if (wl == Plants::invalid) {
                    error_condition = F("Invalid sensor state");
                    state = auto_mode_b;
                    switch_to(error);
                }
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > plants.countPlants())) {
                    error_condition = F("Invalid plant ID!");
                    switch_to(error);
                } else {
                    selected_plants.set(selected_id - 1);
                    menu_entered_digits = "";
                    switch_to(auto_plant);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == fullauto_fert) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(auto_mode_a);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                selected_plants.clear();
                switch_to(fullauto_plant);
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > plants.countFertilizers())) {
                    error_condition = F("Invalid fert. ID!");
                    switch_to(error);
                } else {
                    selected_ferts.set(selected_id - 1);
                    menu_entered_digits = "";
                    switch_to(fullauto_fert);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == fullauto_plant) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                selected_ferts.clear();
                switch_to(fullauto_fert);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                if (selected_plants.countSet() <= 0) {
                    // user has not selected a plant yet...
                    return;
                }

                // check if we need to run fertilizers
                if (selected_ferts.countSet() > 0) {
                    // stirr before pumping fertilizers
                    for (int i = 0; i < STIRRER_COUNT; i++) {
                        plants.startAux(i);
                    }
                    selected_id = 1;
                    selected_time = AUTO_STIRR_RUNTIME;
                    start_time = millis();
                    switch_to(fullauto_stirr_run);
                } else {
                    // immediately continue with filling tank
                    auto wl = plants.getWaterlevel();
                    if ((wl != Plants::full) && (wl != Plants::invalid)) {
                        // if the waterlevel is currently empty, we
                        // set a flag to record the time to fill
                        filling_started_empty = (wl == Plants::empty);

                        plants.openWaterInlet();
                        selected_id = plants.countPlants() + 1;
                        selected_time = MAX_TANK_FILL_TIME;
                        start_time = millis();
                        switch_to(fullauto_tank_run);
                    } else if (wl == Plants::full) {
                        // check if kickstart is required for this
                        bool need_kickstart = false;
                        for (int i = 0; i < plants.countPlants(); i++) {
                            if (selected_plants.isSet(i)) {
                                if (plants.getKickstart()->getPinNumber(i) >= 0) {
                                    need_kickstart = true;
                                }
                            }
                        }

                        // start kickstart/valve as needed
                        for (int i = 0; i < plants.countPlants(); i++) {
                            if (selected_plants.isSet(i)) {
                                plants.startPlant(i, need_kickstart);
                            }
                        }

                        // for recording the flowrate
                        watering_started_full = (wl == Plants::full);

                        selected_time = MAX_AUTO_PLANT_RUNTIME;
                        start_time = millis();
                        if (need_kickstart) {
                            switch_to(fullauto_kickstart_run);
                        } else {
                            switch_to(fullauto_plant_run);
                        }
                    } else if (wl == Plants::invalid) {
                        error_condition = F("Invalid sensor state");
                        state = auto_mode_a;
                        switch_to(error);
                    }
                }
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > plants.countPlants())) {
                    error_condition = F("Invalid plant ID!");
                    switch_to(error);
                } else {
                    selected_plants.set(selected_id - 1);
                    menu_entered_digits = "";
                    switch_to(fullauto_plant);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if ((state == auto_done) || (state == fullauto_done)) {
        switch_to(auto_mode_a);
    } else if (state == menu_pumps) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_b);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }

            selected_id = number_input();

            if ((selected_id <= 0) || (selected_id > plants.countFertilizers())) {
                error_condition = F("Invalid pump ID!");
                switch_to(error);
            } else {
                switch_to(menu_pumps_time);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == fillnwater_plant) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(auto_mode_a);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                if (selected_plants.countSet() <= 0) {
                    return;
                }

                auto wl = plants.getWaterlevel();
                if ((wl != Plants::full) && (wl != Plants::invalid)) {
                    // if the waterlevel is currently empty, we
                    // set a flag to record the time to fill
                    filling_started_empty = (wl == Plants::empty);

                    plants.openWaterInlet();
                    selected_id = plants.countPlants() + 1;
                    selected_time = MAX_TANK_FILL_TIME;
                    start_time = millis();
                    switch_to(fillnwater_tank_run);
                } else if (wl == Plants::full) {
                    // check if kickstart is required for this
                    bool need_kickstart = false;
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            if (plants.getKickstart()->getPinNumber(i) >= 0) {
                                need_kickstart = true;
                            }
                        }
                    }

                    // start kickstart/valve as needed
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i, need_kickstart);
                        }
                    }

                    // for recording the flowrate
                    watering_started_full = (wl == Plants::full);

                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    if (need_kickstart) {
                        switch_to(fillnwater_kickstart_run);
                    } else {
                        switch_to(fillnwater_plant_run);
                    }
                } else if (wl == Plants::invalid) {
                    error_condition = F("Invalid sensor state");
                    state = auto_mode_a;
                    switch_to(error);
                }
            } else {
                selected_id = number_input();
                if ((selected_id <= 0) || (selected_id > plants.countPlants())) {
                    error_condition = F("Invalid plant ID!");
                    switch_to(error);
                } else {
                    selected_plants.set(selected_id - 1);
                    menu_entered_digits = "";
                    switch_to(fillnwater_plant);
                }
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == fillnwater_tank_run) {
        plants.abort();
        stop_time = millis();
        auto wl = plants.getWaterlevel();
        if ((wl != Plants::empty) && (wl != Plants::invalid)) {
            // check if kickstart is required for this
            bool need_kickstart = false;
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    if (plants.getKickstart()->getPinNumber(i) >= 0) {
                        need_kickstart = true;
                    }
                }
            }

            // start kickstart/valve as needed
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    plants.startPlant(i, need_kickstart);
                }
            }

            watering_started_full = (wl == Plants::full);

            selected_time = MAX_AUTO_PLANT_RUNTIME;
            start_time = millis();
            if (need_kickstart) {
                switch_to(fillnwater_kickstart_run);
            } else {
                switch_to(fillnwater_plant_run);
            }
        } else if (wl == Plants::empty) {
            switch_to(auto_mode_a);
        } else if (wl == Plants::invalid) {
            error_condition = F("Invalid sensor state");
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
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_pumps);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }

            selected_time = number_input();
            
            if ((selected_time <= 0) || (selected_time > MAX_PUMP_RUNTIME)) {
                error_condition = F("Invalid time range!");
                switch_to(error);
            } else {
                switch_to(menu_pumps_go);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
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
                error_condition = F("Invalid sensor state");
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
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_b);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }

            selected_id = number_input();

            if ((selected_id <= 0) || (selected_id > (plants.countPlants() + 1))) {
                error_condition = F("Invalid valve ID!");
                switch_to(error);
            } else {
                switch_to(menu_valves_time);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == menu_valves_time) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_valves);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }

            selected_time = number_input();

            if ((selected_time <= 0) || (selected_time > MAX_VALVE_RUNTIME)) {
                error_condition = F("Invalid time range!");
                switch_to(error);
            } else {
                switch_to(menu_valves_go);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
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
                    // TODO support testing kickstart
                    plants.startPlant(selected_id - 1, false);
                }

                switch_to(menu_valves_run);
            } else if (wl == Plants::full) {
                stop_time = millis();
                switch_to(menu_valves_done);
            } else if (wl == Plants::invalid) {
                error_condition = F("Invalid sensor state");
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
    } else if (state == menu_aux) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_c);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }

            selected_id = number_input();

            if ((selected_id <= 0) || (selected_id > plants.countAux())) {
                error_condition = F("Invalid valve ID!");
                switch_to(error);
            } else {
                switch_to(menu_aux_time);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == menu_aux_time) {
        if (n == -1) {
            if (db.hasDigits()) {
                backspace();
                db.removeDigit();
                if (menu_entered_digits.length() > 0) {
                    menu_entered_digits.remove(menu_entered_digits.length() - 1);
                    switch_to(state);
                }
            } else {
                switch_to(menu_aux);
            }
        } else if (n == -2) {
            if (!db.hasDigits()) {
                return;
            }

            selected_time = number_input();

            if ((selected_time <= 0) || (selected_time > MAX_AUX_RUNTIME)) {
                error_condition = F("Invalid time range!");
                switch_to(error);
            } else {
                switch_to(menu_aux_go);
            }
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
                menu_entered_digits += String(n);
                switch_to(state);
            } else {
                backspace();
            }
        }
    } else if (state == menu_aux_go) {
        if (n == -2) {
            start_time = millis();
            last_animation_time = start_time;
            plants.startAux(selected_id - 1);
            switch_to(menu_aux_run);
        } else {
            switch_to(menu_aux_time);
        }
    } else if (state == menu_aux_run) {
        plants.abort();
        stop_time = millis();
        switch_to(menu_aux_done);
    } else if (state == menu_aux_done) {
        switch_to(menu_c);
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
    if ((state == menu_pumps_run) || (state == menu_valves_run)
            || (state == menu_aux_run) || (state == fullauto_stirr_run)
            || (state == auto_stirr_run)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            // stop if timeout has been reached
            plants.abort();
            stop_time = millis();
            if (state == menu_pumps_run) {
                switch_to(menu_pumps_done);
            } else if (state == menu_valves_run) {
                switch_to(menu_valves_done);
            } else if (state == menu_aux_run) {
                switch_to(menu_aux_done);
            } else if (state == fullauto_stirr_run) {
                if (selected_ferts.countSet() > 0) {
                    selected_time = auto_pump_runtime[0];
                    for (int i = 0; i < plants.countFertilizers(); i++) {
                        if (auto_pump_runtime[i] > selected_time) {
                            selected_time = auto_pump_runtime[i];
                        }

                        if (selected_ferts.isSet(i)) {
                            plants.startFertilizer(i);
                        }
                    }

                    start_time = millis();
                    switch_to(fullauto_fert_run);
                } else {
                    auto wl = plants.getWaterlevel();
                    if ((wl != Plants::full) && (wl != Plants::invalid)) {
                        // if the waterlevel is currently empty, we
                        // set a flag to record the time to fill
                        filling_started_empty = (wl == Plants::empty);

                        plants.openWaterInlet();
                        selected_id = plants.countPlants() + 1;
                        selected_time = MAX_TANK_FILL_TIME;
                        start_time = millis();
                        switch_to(fullauto_tank_run);
                    } else if (wl == Plants::full) {
                        // check if kickstart is required for this
                        bool need_kickstart = false;
                        for (int i = 0; i < plants.countPlants(); i++) {
                            if (selected_plants.isSet(i)) {
                                if (plants.getKickstart()->getPinNumber(i) >= 0) {
                                    need_kickstart = true;
                                }
                            }
                        }

                        // start kickstart/valve as needed
                        for (int i = 0; i < plants.countPlants(); i++) {
                            if (selected_plants.isSet(i)) {
                                plants.startPlant(i, need_kickstart);
                            }
                        }

                        watering_started_full = (wl == Plants::full);

                        selected_time = MAX_AUTO_PLANT_RUNTIME;
                        start_time = millis();
                        if (need_kickstart) {
                            switch_to(fullauto_kickstart_run);
                        } else {
                            switch_to(fullauto_plant_run);
                        }
                    } else if (wl == Plants::invalid) {
                        plants.abort();
                        error_condition = F("Invalid sensor state");
                        state = fullauto_fert;
                        switch_to(error);
                    }
                }
            } else {
                switch_to(auto_done);
            }
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
            error_condition = F("Invalid sensor state");
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
            error_condition = F("Invalid sensor state");
            state = menu_valves;
            switch_to(error);
        }
    }
#endif // CHECK_SENSORS_VALVE_PUMP_MENU_EMPTY

    // kickstart states
    if ((state == auto_plant_kickstart_run) || (state == fillnwater_kickstart_run)
            || (state == fullauto_kickstart_run) || (state == fullauto_kickstart_purge_run)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= KICKSTART_RUNTIME) {
            // kickstart is done, switch over to valves
            plants.abort();
            selected_time = MAX_AUTO_PLANT_RUNTIME;
            start_time = millis();

            // start required valves
            for (int i = 0; i < plants.countPlants(); i++) {
                if (selected_plants.isSet(i)) {
                    plants.startPlant(i, false);
                }
            }

            if (state == auto_plant_kickstart_run) {
                switch_to(auto_plant_run);
            } else if (state == fillnwater_kickstart_run) {
                switch_to(fillnwater_plant_run);
            } else if (state == fullauto_kickstart_run) {
                switch_to(fullauto_plant_run);
            } else {
                switch_to(fullauto_plant_purge_run);
            }
        } else if ((millis() - last_animation_time) >= 500) {
            // update animation if needed
            last_animation_time = millis();
            switch_to(state);
        }
    }

    if (state == fullauto_fert_run) {
        unsigned long runtime = millis() - start_time;
        for (int i = 0; i < plants.countFertilizers(); i++) {
            if (selected_ferts.isSet(i)) {
                if ((runtime / 1000UL) >= auto_pump_runtime[i]) {
                    plants.stopFertilizer(i);
                    selected_ferts.set(i, false);
                }
            }
        }

        if ((millis() - last_animation_time) >= 500) {
            // update animation if needed
            last_animation_time = millis();
            switch_to(state);
        }

        if (selected_ferts.countSet() <= 0) {
            plants.abort();

            auto wl = plants.getWaterlevel();
            if ((wl != Plants::full) && (wl != Plants::invalid)) {
                // if the waterlevel is currently empty, we
                // set a flag to record the time to fill
                filling_started_empty = (wl == Plants::empty);

                plants.openWaterInlet();
                selected_id = plants.countPlants() + 1;
                selected_time = MAX_TANK_FILL_TIME;
                start_time = millis();
                switch_to(fullauto_tank_run);
            } else if (wl == Plants::full) {
                // check if kickstart is required for this
                bool need_kickstart = false;
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        if (plants.getKickstart()->getPinNumber(i) >= 0) {
                            need_kickstart = true;
                        }
                    }
                }

                // start kickstart/valve as needed
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        plants.startPlant(i, need_kickstart);
                    }
                }

                // for recording the flowrate
                watering_started_full = (wl == Plants::full);

                selected_time = MAX_AUTO_PLANT_RUNTIME;
                start_time = millis();
                if (need_kickstart) {
                    switch_to(fullauto_kickstart_run);
                } else {
                    switch_to(fullauto_plant_run);
                }
            } else if (wl == Plants::invalid) {
                error_condition = F("Invalid sensor state");
                state = auto_mode_a;
                switch_to(error);
            }
        }
    }

    // states that fill the tank up
    if ((state == auto_fert_run) || (state == auto_tank_run)
            || (state == fullauto_tank_run) || (state == fullauto_tank_purge_run)
            || (state == fillnwater_tank_run)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            // stop if timeout has been reached
            plants.abort();
            stop_time = millis();
            if ((state == fillnwater_tank_run)
                    || (state == fullauto_tank_run)
                    || (state == fullauto_tank_purge_run)) {
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::empty) && (wl != Plants::invalid)) {
                    // check if kickstart is required for this
                    bool need_kickstart = false;
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            if (plants.getKickstart()->getPinNumber(i) >= 0) {
                                need_kickstart = true;
                            }
                        }
                    }

                    // start kickstart/valve as needed
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i, need_kickstart);
                        }
                    }

                    watering_started_full = (wl == Plants::full);

                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    if (need_kickstart) {
                        if (state == fillnwater_tank_run) {
                            switch_to(fillnwater_kickstart_run);
                        } else if (state == fullauto_tank_run) {
                            switch_to(fullauto_kickstart_run);
                        } else {
                            switch_to(fullauto_kickstart_purge_run);
                        }
                    } else {
                        if (state == fillnwater_tank_run) {
                            switch_to(fillnwater_plant_run);
                        } else if (state == fullauto_tank_run) {
                            switch_to(fullauto_plant_run);
                        } else {
                            switch_to(fullauto_plant_purge_run);
                        }
                    }
                } else if (wl == Plants::empty) {
                    stop_time = millis();
                    switch_to(auto_done);
                } else if (wl == Plants::invalid) {
                    error_condition = F("Invalid sensor state");
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
            if ((state == fillnwater_tank_run)
                    || (state == fullauto_tank_run)
                    || (state == fullauto_tank_purge_run)) {
                // record time to fill here, if we started with
                // an empty tank at the start of filling
                if (filling_started_empty) {
                    unsigned long time_to_fill = stop_time - start_time;
                    debug.print("Filling tank took ");
                    debug.print(String(time_to_fill));
                    debug.println("ms");

#if defined(PLATFORM_ESP)
                    wifi_write_database(time_to_fill, "calibrated_filling", -1);
#endif // PLATFORM_ESP
                }

                // check if kickstart is required for this
                bool need_kickstart = false;
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        if (plants.getKickstart()->getPinNumber(i) >= 0) {
                            need_kickstart = true;
                        }
                    }
                }

                // start kickstart/valve as needed
                for (int i = 0; i < plants.countPlants(); i++) {
                    if (selected_plants.isSet(i)) {
                        plants.startPlant(i, need_kickstart);
                    }
                }

                watering_started_full = (wl == Plants::full);

                selected_time = MAX_AUTO_PLANT_RUNTIME;
                start_time = millis();
                if (need_kickstart) {
                    if (state == fillnwater_tank_run) {
                        switch_to(fillnwater_kickstart_run);
                    } else if (state == fullauto_tank_run) {
                        switch_to(fullauto_kickstart_run);
                    } else {
                        switch_to(fullauto_kickstart_purge_run);
                    }
                } else {
                    if (state == fillnwater_tank_run) {
                        switch_to(fillnwater_plant_run);
                    } else if (state == fullauto_tank_run) {
                        switch_to(fullauto_plant_run);
                    } else {
                        switch_to(fullauto_plant_purge_run);
                    }
                }
            } else {
                switch_to(auto_done);
            }
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = F("Invalid sensor state");
            state = auto_mode_a;
            switch_to(error);
        }
    }

    // states that empty the tank
    if ((state == auto_plant_run) || (state == fillnwater_plant_run)
            || (state == fullauto_plant_run) || (state == fullauto_plant_overrun)
            || (state == fullauto_plant_purge_run)
            || (state == fullauto_plant_purge_overrun)) {
        unsigned long runtime = millis() - start_time;
        if ((runtime / 1000UL) >= selected_time) {
            if (state == fullauto_plant_run) {
                start_time = millis();
                selected_time = OVERRUN_RUNTIME;
                switch_to(fullauto_plant_overrun);
            } else if (state == fullauto_plant_overrun) {
                plants.abort();
                stop_time = millis();
                auto wl = plants.getWaterlevel();
                if ((wl != Plants::full) && (wl != Plants::invalid)) {
                    // if the waterlevel is currently empty, we
                    // set a flag to record the time to fill
                    filling_started_empty = (wl == Plants::empty);

                    plants.openWaterInlet();
                    selected_id = plants.countPlants() + 1;
                    selected_time = PURGE_FILL_RUNTIME;
                    start_time = millis();
                    switch_to(fullauto_tank_purge_run);
                } else if (wl == Plants::full) {
                    // check if kickstart is required for this
                    bool need_kickstart = false;
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            if (plants.getKickstart()->getPinNumber(i) >= 0) {
                                need_kickstart = true;
                            }
                        }
                    }

                    // start kickstart/valve as needed
                    for (int i = 0; i < plants.countPlants(); i++) {
                        if (selected_plants.isSet(i)) {
                            plants.startPlant(i, need_kickstart);
                        }
                    }

                    // for recording the flowrate
                    watering_started_full = (wl == Plants::full);

                    selected_time = MAX_AUTO_PLANT_RUNTIME;
                    start_time = millis();
                    if (need_kickstart) {
                        switch_to(fullauto_kickstart_purge_run);
                    } else {
                        switch_to(fullauto_plant_purge_run);
                    }
                } else if (wl == Plants::invalid) {
                    error_condition = F("Invalid sensor state");
                    state = auto_mode_a;
                    switch_to(error);
                }
            } else if (state == fullauto_plant_purge_run) {
                start_time = millis();
                selected_time = OVERRUN_RUNTIME;
                switch_to(fullauto_plant_purge_overrun);
            } else if (state == fullauto_plant_purge_overrun) {
                plants.abort();
                stop_time = millis();
                switch_to(fullauto_done);
            } else {
                plants.abort();
                stop_time = millis();
                switch_to(auto_done);
            }
        } else if ((millis() - last_animation_time) >= 500) {
            // update animation if needed
            last_animation_time = millis();
            switch_to(state);
        }

        if ((state == fullauto_plant_overrun) || (state == fullauto_plant_purge_overrun)) {
            // overrun should not exit when sensor returns empty
            return;
        }

        // check water level state
        auto wl = plants.getWaterlevel();
        if (wl == Plants::empty) {
            if ((state == auto_plant_run) || (state == fillnwater_plant_run)) {
                plants.abort();
                stop_time = millis();
            }

            if ((state == auto_plant_run) || (state == fillnwater_plant_run)
            || (state == fullauto_plant_run)) {
                // if we started watering with a full tank
                // and then finished watering when it was empty
                // and we were only watering a single plant
                // look at this as a "calibration run" and record
                // the time it took to empty the tank
                if (watering_started_full && (selected_plants.countSet() == 1)) {
                    unsigned long time_to_water = stop_time - start_time;
                    debug.print("Watering plant ");
                    debug.print(selected_plants.getFirstSet() + 1);
                    debug.print(" with the complete tank took ");
                    debug.print(String(time_to_water));
                    debug.println("ms");

#if defined(PLATFORM_ESP)
                    wifi_write_database(time_to_water, "calibrated_watering", selected_plants.getFirstSet() + 1);
#endif // PLATFORM_ESP
                }
            }

            if ((state == auto_plant_run) || (state == fillnwater_plant_run)) {
                switch_to(auto_done);
            } else if (state == fullauto_plant_run) {
                start_time = millis();
                selected_time = OVERRUN_RUNTIME;
                switch_to(fullauto_plant_overrun);
            } else if (state == fullauto_plant_purge_run) {
                start_time = millis();
                selected_time = OVERRUN_RUNTIME;
                switch_to(fullauto_plant_purge_overrun);

            }
        } else if (wl == Plants::invalid) {
            plants.abort();
            error_condition = F("Invalid sensor state");
            state = auto_mode_a;
            switch_to(error);
        }
    }

    // timeout in user-input states
    if ((state == menu_a) || (state == menu_b) || (state == menu_c)
            || (state == automation_mode) || (state == auto_done)
            || (state == auto_mode_a) || (state == auto_mode_b)
            || (state == auto_fert_a) || (state == auto_fert_b)
            || (state == auto_plant) || (state == fillnwater_plant)
            || (state == menu_pumps) || (state == menu_pumps_time)
            || (state == menu_pumps_go) || (state == menu_pumps_done)
            || (state == menu_valves) || (state == menu_valves_time)
            || (state == menu_valves_go) || (state == menu_valves_done)
            || (state == menu_aux) || (state == menu_aux_time)
            || (state == menu_aux_go) || (state == menu_aux_done)
            || (state == fullauto_fert) || (state == fullauto_plant)
            || (state == fullauto_done)) {
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

        menu_entered_digits = "";
    }
    
    if (s == init) {
        String a = String(F("- Giess-o-mat V")) + FIRMWARE_VERSION + String(F(" -"));

#if (LOCK_COUNT > 0) && defined(DOOR_LOCK_PIN)
        String b = String(F("PIN: ")) + menu_entered_digits;
        print(a.c_str(),
              "* or # to enter menu",
              "Enter PIN for locks:",
              b.c_str(),
              3);
#else
        print(a.c_str(),
              "Usage:  Enter number",
              "* Delete prev. digit",
              "# Execute input num.",
              -1);
#endif
    } else if (s == door_select) {
        String a = String(F("(Input 1 to ")) + String(LOCK_COUNT) + String(F(")"));
        String b = String(F("Door: ")) + menu_entered_digits;

        print("- Select Door Lock -",
              "Leave empty if done!",
              a.c_str(),
              b.c_str(),
              3);
    } else if (s == menu_a) {
        print("----- Menu 1/3 -----",
              "1: Manual Operation",
              "2: Automation",
              "#: Go to page 2/3...",
              -1);
    } else if (s == menu_b) {
        print("----- Menu 2/3 -----",
              "3: Fertilizer pumps",
              "4: Outlet valves",
              "#: Go to page 3/3...",
              -1);
    } else if (s == menu_c) {
        print("----- Menu 3/3 -----",
              "5: Aux. Outputs",
#if (LOCK_COUNT > 0) && !defined(DOOR_LOCK_PIN)
              "6: Door Locks",
#else
              "",
#endif
              "#: Go to page 1/3...",
              -1);
    } else if (state == automation_mode) {
        // TODO
        print("---- Automation ----",
              "TODO NOT IMPLEMENTED",
              "TODO NOT IMPLEMENTED",
              "TODO NOT IMPLEMENTED",
              -1);
    } else if (s == auto_mode_a) {
        print("---- Manual 1/3 ----",
              "1: Full-Auto Mode",
              "2: Add Fertilizer",
              "#: Go to page 2/3...",
              -1);
    } else if (s == auto_mode_b) {
        print("---- Manual 2/3 ----",
              "3: Fill 'n' Water",
              "4: Fill Reservoir",
              "#: Go to page 3/3...",
              -1);
    } else if (s == auto_mode_c) {
        print("---- Manual 3/3 ----",
              "5: Water a plant",
              "",
              "#: Go to page 1/3...",
              -1);
    } else if (s == auto_fert_a) {
        print("-- Fertilizer 1/2 --",
              "1: Vegetation Phase",
              "2: Bloom Phase",
              "#: Go to page 2/2...",
              -1);
    } else if (s == auto_fert_b) {
        print("-- Fertilizer 2/2 --",
              "3: Special Fert.",
              "4: Run Stirrer",
              "#: Go to page 1/2...",
              -1);
    } else if ((s == auto_fert_run) || (s == fullauto_fert_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');
        
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
    } else if ((s == auto_stirr_run) || (s == fullauto_stirr_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');

        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }

        print("----- Stirring -----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if ((s == auto_tank_run) || (s == fillnwater_tank_run) || (s == fullauto_tank_run) || (s == fullauto_tank_purge_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');
        
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
    } else if ((s == auto_plant) || (s == fillnwater_plant) || (s == fullauto_plant)) {
        String a = String(F("(Input 1 to ")) + String(plants.countPlants()) + String(F(")"));
        String b = String(F("Plant: ")) + menu_entered_digits;

        print("--- Select Plant ---",
              "Leave empty if done!",
              a.c_str(),
              b.c_str(),
              3);
    } else if (s == fullauto_fert) {
        String a = String(F("(Input 1 to ")) + String(plants.countFertilizers()) + String(F(")"));
        String b = String(F("Fert.: ")) + menu_entered_digits;

        print("--- Select Fert. ---",
              "Leave empty if done!",
              a.c_str(),
              b.c_str(),
              3);
    } else if ((s == auto_plant_kickstart_run) || (s == fillnwater_kickstart_run) || (s == fullauto_kickstart_run) || (s == fullauto_kickstart_purge_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(KICKSTART_RUNTIME) + String('s');

        unsigned long anim = runtime * 20UL / (KICKSTART_RUNTIME * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }

        print("---- Kick-Start ----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if ((s == auto_plant_run) || (s == fillnwater_plant_run) || (s == fullauto_plant_run) || (s == fullauto_plant_purge_run)) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');
        
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
    } else if ((s == fullauto_plant_overrun) || (s == fullauto_plant_purge_overrun)) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');

        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i < anim; i++) {
            b += '#';
        }

        print("-- Emptying lines --",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if ((s == auto_done) || (s == fullauto_done)) {
        String a = String(F("after ")) + String((stop_time - start_time) / 1000UL) + String(F("s."));
        
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
                    wifi_write_database(runtime / 1000, "plant", i + 1);
                }
            }
        } else if (old_state == auto_fert_run) {
            wifi_write_database(runtime / 1000, "fertilizer", selected_id);
        }
#endif // PLATFORM_ESP
    } else if (s == menu_pumps) {
        String a = String(F("(Input 1 to ")) + String(plants.countFertilizers()) + String(F(")"));
        String b = String(F("Pump: ")) + menu_entered_digits;
        
        print("------- Pump -------",
              "Please select pump",
              a.c_str(),
              b.c_str(),
              3);
    } else if (s == menu_pumps_time) {
        String header = String(F("------ Pump ")) + String(selected_id) + String(F(" ------"));
        String b = String(F("Runtime: ")) + menu_entered_digits;
        
        print(header.c_str(),
              "Please set runtime",
              "(Input in seconds)",
              b.c_str(),
              3);
    } else if (s == menu_pumps_go) {
        String a = String(F("Pump No. ")) + String(selected_id);
        String b = String(F("Runtime ")) + String(selected_time) + String('s');
        
        print("----- Confirm? -----",
              a.c_str(),
              b.c_str(),
              "           # Confirm",
              -1);
    } else if (s == menu_pumps_run) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');
        
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
        String a = String(F("after ")) + String((stop_time - start_time) / 1000UL) + String(F("s."));
        
        print("------- Done -------",
              "Dispensing finished",
              a.c_str(),
              "Hit any key for menu",
              -1);

#if defined(PLATFORM_ESP)
        unsigned long runtime = stop_time - start_time;
        wifi_write_database(runtime / 1000, "fertilizer", selected_id);
#endif // PLATFORM_ESP
    } else if (s == menu_valves) {
        String a = String(F("(Input 1 to ")) + String(plants.countPlants() + 1) + String(F(")"));
        String b = String(F("Valve: ")) + menu_entered_digits;
        
        print("------ Valves ------",
              "Please select valve",
              a.c_str(),
              b.c_str(),
              3);
    } else if (s == menu_valves_time) {
        String header = String(F("----- Valve  ")) + String(selected_id) + String(F(" -----"));
        String b = String(F("Runtime: ")) + menu_entered_digits;
        
        print(header.c_str(),
              "Please set runtime",
              "(Input in seconds)",
              b.c_str(),
              3);
    } else if (s == menu_valves_go) {
        String a = String(F("Valve No. ")) + String(selected_id);
        String b = String(F("Runtime ")) + String(selected_time) + String('s');
        
        print("----- Confirm? -----",
              a.c_str(),
              b.c_str(),
              "           # Confirm",
              -1);
    } else if (s == menu_valves_run) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');
        
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
        String a = String(F("after ")) + String((stop_time - start_time) / 1000UL) + String(F("s."));
        
        print("------- Done -------",
              "Dispensing finished",
              a.c_str(),
              "Hit any key for menu",
              -1);

#if defined(PLATFORM_ESP)
        unsigned long runtime = stop_time - start_time;
        if (selected_id <= plants.countPlants()) {
            wifi_write_database(runtime / 1000, "plant", selected_id);
        }
#endif // PLATFORM_ESP
    } else if (s == menu_aux) {
        String a = String(F("(Input 1 to ")) + String(plants.countAux()) + String(F(")"));
        String b = String(F("Aux.: ")) + menu_entered_digits;

        print("------- Aux. -------",
              "Please select aux.",
              a.c_str(),
              b.c_str(),
              3);
    } else if (s == menu_aux_time) {
        String header = String(F("------ Aux  ")) + String(selected_id) + String(F(" ------"));
        String b = String(F("Runtime: ")) + menu_entered_digits;

        print(header.c_str(),
              "Please set runtime",
              "(Input in seconds)",
              b.c_str(),
              3);
    } else if (s == menu_aux_go) {
        String a = String(F("Aux No. ")) + String(selected_id);
        String b = String(F("Runtime ")) + String(selected_time) + String('s');

        print("----- Confirm? -----",
              a.c_str(),
              b.c_str(),
              "           # Confirm",
              -1);
    } else if (s == menu_aux_run) {
        unsigned long runtime = millis() - start_time;
        String a = String(F("Time: ")) + String(runtime / 1000UL) + String(F("s / ")) + String(selected_time) + String('s');

        unsigned long anim = runtime * 20UL / (selected_time * 1000UL);
        String b;
        for (unsigned long i = 0; i <= anim; i++) {
            b += '#';
        }

        print("----- Stirring -----",
              a.c_str(),
              b.c_str(),
              "Hit any key to stop!",
              -1);
    } else if (s == menu_aux_done) {
        String a = String(F("after ")) + String((stop_time - start_time) / 1000UL) + String(F("s."));

        print("------- Done -------",
              "Aux. run finished",
              a.c_str(),
              "Hit any key for menu",
              -1);
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

#endif
