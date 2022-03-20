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
#define BACK_TO_IDLE_TIMEOUT (5UL * 60UL * 1000UL)

// in seconds
#define MAX_TANK_FILL_TIME (70)
#define AUTO_PUMP_RUNTIME 4
#define AUTO_STIRR_RUNTIME 60
#define MAX_AUTO_PLANT_RUNTIME (35 * 60)
#define MAX_PUMP_RUNTIME 30
#define MAX_VALVE_RUNTIME (45 * 60)
#define MAX_AUX_RUNTIME (5 * 60)
#define KICKSTART_RUNTIME 10

// Sketch version
#define FIRMWARE_VERSION "0.5"

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

//#define ENABLE_GPIO_TEST
#define GPIO_TEST_INTERVAL 4000
#define GPIO_TEST_DELAY 200

// InfluxDB settings
#define ENABLE_INFLUXDB_LOGGING
#define INFLUXDB_HOST "10.23.42.14"
#define INFLUXDB_PORT 8086
#define INFLUXDB_DATABASE "giessomat"

#define DOOR_LOCK_PIN 4223
#define DOOR_LOCK_PIN_MAX_DIGITS 6
#define DOOR_LOCK_ON_TIME 200 /* in ms */
#define DOOR_LOCK_NEXT_DELAY 100 /* in ms */

#endif // _CONFIG_H_
