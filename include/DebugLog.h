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
