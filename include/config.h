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

#ifndef _CONFIG_H_
#define _CONFIG_H_

//#define DEBUG_WAIT_FOR_SERIAL_CONN

#define DEBUG_ENABLE_LCD_OUTPUT_ON_SERIAL
#define DEBUG_ENABLE_KEYPAD_INPUT_ON_SERIAL

// in milliseconds
#define DISPLAY_BACKLIGHT_TIMEOUT (5UL * 60UL * 1000UL)

// in seconds
#define MAX_TANK_FILL_TIME 30
#define AUTO_PUMP_RUNTIME 5
#define MAX_AUTO_PLANT_RUNTIME (10 * 60)
#define MAX_PUMP_RUNTIME 30
#define MAX_VALVE_RUNTIME (10 * 60)

// Sketch version
#define FIRMWARE_VERSION "0.2"

// all given in milliseconds
#define SERVER_HANDLE_INTERVAL 10
#define WEBSOCKET_UPDATE_INTERVAL 500
#define LED_BLINK_INTERVAL 500
#define LED_INIT_BLINK_INTERVAL 500
#define LED_CONNECT_BLINK_INTERVAL 250
#define LED_ERROR_BLINK_INTERVAL 100

#define INVERT_SENSOR_BOTTOM
//#define INVERT_SENSOR_TOP

#define CHECK_SENSORS_VALVE_PUMP_MENU_FULL // be careful when uncommenting this!
//#define CHECK_SENSORS_VALVE_PUMP_MENU_EMPTY

#define OWN_I2C_ADDRESS 0x42
#define I2C_BUS_SPEED 400000
#define I2C_BUF_SIZE 32

#endif // _CONFIG_H_
