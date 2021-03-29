#include <Arduino.h>
#include "Statemachine.h"

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

Statemachine::Statemachine(print_fn _print, backspace_fn _backspace)
        : db(7) {
    state = init;
    print = _print;
    backspace = _backspace;
    
    selected_id = 0;
    selected_time = 0;
}

void Statemachine::begin(void) {
    switch_to(init);
}

void Statemachine::input(int n) {
    if (state == init) {
        switch_to(menu);
    } else if (state == menu) {
        if (n == 1) {
            switch_to(menu_auto);
        } else if (n == 2) {
            switch_to(menu_pumps);
        } else if (n == 3) {
            switch_to(menu_valves);
        } else if ((n == -1) || (n == -2)) {
            switch_to(init);
        }
    } else if (state == menu_auto) {
        if ((n == -1) || (n == -2)) {
            switch_to(menu);
        } else if (n == 1) {
            // water only
            
        } else if (n == 2) {
            // with fertilizer
            
        }
    } else if (state == menu_auto_mode) {
        switch_to(menu);
    } else if (state == menu_auto_go) {
        switch_to(menu);
    } else if (state == menu_auto_done) {
        switch_to(menu);
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
            
            // TODO validate
            switch_to(menu_pumps_time);
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
            
            // TODO validate
            switch_to(menu_pumps_go);
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == menu_pumps_go) {
        switch_to(menu);
    } else if (state == menu_pumps_running) {
        switch_to(menu);
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
            
            // TODO validate
            switch_to(menu_valves_time);
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
            
            // TODO validate
            switch_to(menu_valves_go);
        } else {
            if (db.spaceLeft()) {
                db.addDigit(n);
            } else {
                backspace();
            }
        }
    } else if (state == menu_valves_go) {
        switch_to(menu);
    } else if (state == menu_valves_running) {
        switch_to(menu);
    } else if (state == menu_valves_done) {
        switch_to(menu);
    }
}

uint32_t Statemachine::number_input(void) {
    for (int i = 0; i < db.countDigits(); i++) {
        backspace();
    }
    
    uint32_t n = db.getNumber();
    db.clear();
    
    Serial.print("Whole number input: ");
    Serial.println(n);
    
    return n;
}

void Statemachine::act(void) {
    
}

void Statemachine::switch_to(States s) {
    state = s;
    
    if (s == init) {
        print("- Giess-o-mat V0.1 -",
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
    } else if (s == menu_auto) {
        print("------- Auto -------",
              "1: Water only",
              "2: With fertilizer",
              "",
              -1);
    } else if (s == menu_auto_mode) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_auto_go) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_auto_done) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_pumps) {
        print("------- Pump -------",
              "Please select pump",
              "(Input 1 to 3)",
              "Pump: ",
              3);
    } else if (s == menu_pumps_time) {
        print("------ Pump X ------",
              "Please set runtime",
              "(Input in seconds)",
              "Runtime: ",
              3);
    } else if (s == menu_pumps_go) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_pumps_running) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_pumps_done) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_valves) {
        print("------ Valves ------",
              "Please select valve",
              "(Input 1 to 5)",
              "Valve: ",
              3);
    } else if (s == menu_valves_time) {
        print("----- Valve XX -----",
              "Please set runtime",
              "(Input in seconds)",
              "Runtime: ",
              3);
    } else if (s == menu_valves_go) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_valves_running) {
        print("",
              "",
              "",
              "",
              -1);
    } else if (s == menu_valves_done) {
        print("",
              "",
              "",
              "",
              -1);
    }
}
