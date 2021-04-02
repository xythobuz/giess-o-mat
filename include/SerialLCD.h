#ifndef _SERIAL_LCD_H_
#define _SERIAL_LCD_H_

#ifdef FUNCTION_UI

#if defined(PLATFORM_AVR)
#include <SendOnlySoftwareSerial.h>
#elif defined(PLATFORM_ESP)
#include <SoftwareSerial.h>
#else
#error platform not supported
#endif

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
#if defined(PLATFORM_AVR)
    SendOnlySoftwareSerial *lcd;
#elif defined(PLATFORM_ESP)
    SoftwareSerial *lcd;
#endif
};

#endif // FUNCTION_UI

#endif // _SERIAL_LCD_H_
