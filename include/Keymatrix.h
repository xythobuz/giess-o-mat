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
