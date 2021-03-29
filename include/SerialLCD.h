#ifndef _SERIAL_LCD_H_
#define _SERIAL_LCD_H_

#include <SendOnlySoftwareSerial.h>

class SerialLCD {
public:
    SerialLCD(int tx_pin);
    ~SerialLCD(void);
    
    void init(void);
    void clear(void);
    void setBacklight(uint8_t val);
    
    // 0 no cursor, 1 underline, 2 blinking, 3 both
    void cursor(int style);
    
    void position(int line, int col);
    
    void write(const char *text);
    void write(int line, const char *text);
    void write(int line, int col, const char *text);
    
private:
    SendOnlySoftwareSerial *lcd;
};

#endif // _SERIAL_LCD_H_
