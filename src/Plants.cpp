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

#include "DebugLog.h"
#include "Plants.h"
#include "config.h"
#include "config_pins.h"
    
// valves: no of plants + 1 for water inlet
// pumps: no of fertilizers
// switches: 2, low and high level
Plants::Plants(int valve_count, int pump_count, int switch_count) :
        valves(valve_count), pumps(pump_count), switches(switch_count) {
}

    
GPIOBank *Plants::getValves(void) {
    return &valves;
}

GPIOBank *Plants::getPumps(void) {
    return &pumps;
}

GPIOBank *Plants::getSwitches(void) {
    return &switches;
}

void Plants::setValvePins(int pins[]) {
    valves.setPinNumbers(pins);
    valves.setOutput();
    valves.setAll(false);
}

void Plants::setPumpPins(int pins[]) {
    pumps.setPinNumbers(pins);
    pumps.setOutput();
    pumps.setAll(false);
}

void Plants::setSwitchPins(int pins[], bool pullup) {
    switches.setPinNumbers(pins);
    switches.setInput(pullup);
}

void Plants::abort(void) {
    closeWaterInlet();
    stopAllFertilizers();
    stopAllPlants();
}

Plants::Waterlevel Plants::getWaterlevel(void) {
    bool low = switches.getPin(0);
    bool high = switches.getPin(1);
    
#ifdef INVERT_SENSOR_BOTTOM
    low = !low;
#endif // INVERT_SENSOR_BOTTOM
    
#ifdef INVERT_SENSOR_TOP
    high = !high;
#endif // INVERT_SENSOR_TOP
    
    if ((!low) && (!high)) {
        return empty;
    } else if (low && (!high)) {
        return inbetween;
    } else if (low && high) {
        return full;
    } else {
        return invalid;
    }
}

void Plants::openWaterInlet(void) {
    debug.println("Plants::openWaterInlet");
    valves.setPin(countPlants(), true);
}

void Plants::closeWaterInlet(void) {
    debug.println("Plants::closeWaterInlet");
    valves.setPin(countPlants(), false);
}

int Plants::countFertilizers(void) {
    return pumps.getSize();
}

void Plants::startFertilizer(int id) {
    debug.print("Plants::startFertilizer ");
    debug.println(id);
    
    if ((id >= 0) && (id < countFertilizers())) {
        pumps.setPin(id, true);
    }
}

void Plants::stopFertilizer(int id) {
    debug.print("Plants::stopFertilizer ");
    debug.println(id);
    
    if ((id >= 0) && (id < countFertilizers())) {
        pumps.setPin(id, false);
    }
}

void Plants::stopAllFertilizers(void) {
    for (int i = 0; i < countFertilizers(); i++) {
        stopFertilizer(i);
    }
}

int Plants::countPlants(void) {
    return valves.getSize() - 1;
}

void Plants::startPlant(int id) {
    debug.print("Plants::startPlant ");
    debug.println(id);
    
    if ((id >= 0) && (id < countPlants())) {
        valves.setPin(id, true);
    }
}

void Plants::stopPlant(int id) {
    debug.print("Plants::stopPlant ");
    debug.println(id);
    
    if ((id >= 0) && (id < countPlants())) {
        valves.setPin(id, false);
    }
}

void Plants::stopAllPlants(void) {
    for (int i = 0; i < countPlants(); i++) {
        stopPlant(i);
    }
}
