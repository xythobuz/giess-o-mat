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

#ifndef _PLANTS_H_
#define _PLANTS_H_

#include "GPIOBank.h"

class Plants {
public:
    enum Waterlevel {
        empty,
        inbetween,
        full,
        invalid
    };
    
    // valves: no of plants + 1 for water inlet
    // pumps: no of fertilizers
    // switches: 2, low and high level
    Plants(int valve_count, int pump_count, int switch_count);
    
    void setValvePins(int pins[]);
    void setPumpPins(int pins[]);
    void setSwitchPins(int pins[], bool pullup);
    
    void abort(void);
    
    Waterlevel getWaterlevel(void);
    void openWaterInlet(void);
    void closeWaterInlet(void);
    
    int countFertilizers(void);
    void startFertilizer(int id);
    void stopFertilizer(int id);
    void stopAllFertilizers(void);
    
    int countPlants(void);
    void startPlant(int id);
    void stopPlant(int id);
    void stopAllPlants(void);
    
    GPIOBank *getValves(void);
    GPIOBank *getPumps(void);
    GPIOBank *getSwitches(void);
    
private:
    GPIOBank valves;
    GPIOBank pumps;
    GPIOBank switches;
};

extern Plants plants;

#endif // _PLANTS_H_
