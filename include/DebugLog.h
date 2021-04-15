#ifndef _DEBUG_LOG_H_
#define _DEBUG_LOG_H_

#include <Arduino.h>

#ifdef PLATFORM_ESP
#include <CircularBuffer.h>
#define DEBUG_LOG_HISTORY_SIZE 1024
#endif // PLATFORM_ESP

class DebugLog {
public:
#ifdef PLATFORM_ESP
    String getBuffer(void);
#endif // PLATFORM_ESP
    
    void print(String s);
    void print(int n);
    
    void println(void);
    void println(String s);
    void println(int n);
    
private:
    void sendToTargets(String s);
    
#ifdef PLATFORM_ESP
    void addToBuffer(String s);
    
    CircularBuffer<char, DEBUG_LOG_HISTORY_SIZE> buffer;
#endif // PLATFORM_ESP
};

extern DebugLog debug;

#endif // _DEBUG_LOG_H_
