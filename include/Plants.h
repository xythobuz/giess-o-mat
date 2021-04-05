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
