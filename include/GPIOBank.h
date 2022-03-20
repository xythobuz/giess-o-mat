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

#ifndef _GPIO_BANK_H_
#define _GPIO_BANK_H_

void gpio_i2c_init(void);

class GPIOBank {
public:
    GPIOBank(int _size);
    ~GPIOBank(void);
    
    void setPinNumbers(int _pins[]);
    int getPinNumber(int pin);
    void setOutput(void);
    void setInput(bool pullup);
    
    int getSize(void);
    void setPin(int n, bool state);
    void setAll(bool state);
    bool getPin(int n);
    
private:
    int size;
    int *pins;
    bool *out_state;
    bool is_output;
};

#endif // _GPIO_BANK_H_
