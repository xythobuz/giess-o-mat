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

#ifndef _CONFIG_PINS_H_
#define _CONFIG_PINS_H_

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef PLATFORM_AVR

#define BUILTIN_LED_PIN 13

// ----------------------------------------------------------------------------

#ifdef FUNCTION_UI

/*
 * AVR UI
 */

#define SERIAL_LCD_TX_PIN 10

#define KEYMATRIX_ROWS 4
#define KEYMATRIX_COLS 3
#define KEYMATRIX_ROW_PINS 5, 6, 7, 8
#define KEYMATRIX_COL_PINS 2, 3, 4

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

/*
 * AVR Controller
 */

// out 1, out 2, out 3, out 4, in
#define VALVE_COUNT 5
#define VALVE_PINS 10, 11, 12, 14, 15

// a, b, c
#define PUMP_COUNT 3
#define PUMP_PINS 16, 17, 18

// bottom, top
#define SWITCH_COUNT 2
#define SWITCH_PINS 19, 20

// stirrer
#define AUX_COUNT 1
#define AUX_PINS 21

#endif // FUNCTION_CONTROL

#endif // PLATFORM_AVR

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef PLATFORM_ESP

#define BUILTIN_LED_PIN 1

// ----------------------------------------------------------------------------

#ifdef FUNCTION_UI

/*
 * ESP UI
 */

#error configuration not supported

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

/*
 * ESP Controller
 */

/*
 * I2C Port Expander PCF8574(A)
 *
 * Address ranges:
 * PCF8574: 0x20 - 0x27
 * PCF8574A: 0x38 - 0x3F
 */
#define I2C_GPIO_EXPANDER_COUNT 2
#define I2C_GPIO_EXPANDER_ADDR 0x20, 0x21

/*
 * GPIO Numbering Scheme
 * 000 - 099: normal ESP32 GPIOs
 * 100 - 228: consecutive I2C Expander GPIOs
 */

#define PLANT_COUNT 8
#define PLANT_PINS 27, 14, 5, 18, 100, 101, 102, 103

#define INLET_PIN 15

// out 1, out 2, out 3, out 4, in
#define VALVE_COUNT (PLANT_COUNT + 1)
#define VALVE_PINS PLANT_PINS, INLET_PIN

// kickstarting pumps, same count as plants!
// set pin to -1 to disable kickstart for this plant.
#define KICKSTART_PINS -1, -1, -1, -1, 108, 109, 110, 111

// a, b, c
#define PUMP_COUNT 3
#define PUMP_PINS 2, 0, 4

// stirrer, locks
#define STIRRER_COUNT 1
#define LOCK_COUNT 2
#define AUX_COUNT (STIRRER_COUNT + LOCK_COUNT)
#define AUX_PINS 19, 104, 105

// bottom, top
#define SWITCH_COUNT 2
#define SWITCH_PINS 26, 25

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#endif // FUNCTION_CONTROL

#endif // PLATFORM_ESP

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#endif // _CONFIG_PINS_H_
