#ifndef _GPIO_BANK_H_
#define _GPIO_BANK_H_

class GPIOBank {
public:
    GPIOBank(int _size);
    ~GPIOBank(void);
    
    void setPinNumbers(int _pins[]);
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
