#include <Arduino.h>
#include "Plants.h"
    
// valves: no of plants + 1 for water inlet
// pumps: no of fertilizers
// switches: 2, low and high level
Plants::Plants(int valve_count, int pump_count, int switch_count) :
        valves(valve_count), pumps(pump_count), switches(switch_count) {
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
    bool low = !switches.getPin(0);
    bool high = !switches.getPin(1);
    
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
    valves.setPin(countPlants(), true);
}

void Plants::closeWaterInlet(void) {
    valves.setPin(countPlants(), false);
}

int Plants::countFertilizers(void) {
    return pumps.getSize();
}

void Plants::startFertilizer(int id) {
    if ((id >= 0) && (id < countFertilizers())) {
        pumps.setPin(id, true);
    }
}

void Plants::stopFertilizer(int id) {
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
    if ((id >= 0) && (id < countPlants())) {
        valves.setPin(id, true);
    }
}

void Plants::stopPlant(int id) {
    if ((id >= 0) && (id < countPlants())) {
        valves.setPin(id, false);
    }
}

void Plants::stopAllPlants(void) {
    for (int i = 0; i < countPlants(); i++) {
        stopPlant(i);
    }
}
