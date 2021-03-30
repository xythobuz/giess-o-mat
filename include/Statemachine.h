#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include <Arduino.h>

class Statemachine {
public:
    enum States {
        init,
        menu, // auto, pumps, valves
        
        menu_auto, // select plant
        menu_auto_mode, // select mode
        menu_auto_go, // running
        menu_auto_done,
        
        menu_pumps, // selet pump
        menu_pumps_time, // set runtime
        menu_pumps_go, // running
        menu_pumps_running,
        menu_pumps_done,
        
        menu_valves, // select valve
        menu_valves_time, // set runtime
        menu_valves_go, // running
        menu_valves_running,
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
