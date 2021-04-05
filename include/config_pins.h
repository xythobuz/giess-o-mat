#ifndef _CONFIG_PINS_H_
#define _CONFIG_PINS_H_

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef PLATFORM_AVR

#define BUILTIN_LED_PIN 13

// ----------------------------------------------------------------------------

#ifdef FUNCTION_UI

#define SERIAL_LCD_TX_PIN 10

#define KEYMATRIX_ROWS 4
#define KEYMATRIX_COLS 3
#define KEYMATRIX_ROW_PINS 5, 6, 7, 8
#define KEYMATRIX_COL_PINS 2, 3, 4

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

#define VALVE_COUNT 5
#define VALVE_PINS 10, 11, 12, 14, 15

#define PUMP_COUNT 3
#define PUMP_PINS 16, 17, 18

#define SWITCH_COUNT 2
#define SWITCH_PINS 19, 20

#endif // FUNCTION_CONTROL

#endif // PLATFORM_AVR

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef PLATFORM_ESP

#define BUILTIN_LED_PIN 1

// ----------------------------------------------------------------------------

#ifdef FUNCTION_UI

#error configuration not supported

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

// R1 R2 R3 R4  R5 R6 R7 R8
//  4  0  2 15  18  5 17 16

#define VALVE_COUNT 5
#define VALVE_PINS 4, 0, 2, 15, 18

#define PUMP_COUNT 3
#define PUMP_PINS 5, 17, 16

#define SWITCH_COUNT 2
#define SWITCH_PINS 22, 23

#endif // FUNCTION_CONTROL

#endif // PLATFORM_ESP

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#endif // _CONFIG_PINS_H_
