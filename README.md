# Giess-o-mat

Firmware for automatic plant-watering machine, with web interface, LCD and keypad support, using two MCUs communicating via I2C.

For more please also [take a look at Giess-o-mat on my website](https://www.xythobuz.de/giessomat.html).

## Quick Start

If you don't already have it, get [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html).

Before building for an ESP32 or ESP8266 target, execute the following commands, replacing "..." with the values of your network, so you can connect to WiFi.

    echo '#define WIFI_SSID "..."' > include/wifi.h
    echo '#define WIFI_PW "..."' >> include/wifi.h

Then simply run the build for all supported configurations with platformio.

    pio run

You can of course also use pio to flash your targets.

## Hardware

In general, the project consists of two parts, the controller and the ui.
They both communicate via I2C.
In my own setup, I'm running the controller on an ESP32, and the UI on an Arduino Nano.
It is also possible to run the controller with an ESP8266 or an Arduino instead.
You can also run both on one board, but this was up to now only used on Arduino for testing purposes.

Of course you need to take care of voltage levels when connecting different boards.
I'm running the UI with 5V and the ESP32 is using 3.3V.
For safety reasons I added a bi-directional I2C level converter between them.

## Details

The following targets are defined for this project.

| Name          | Board            | Function   | Tested | Usable |
| ------------- | ---------------- | ---------- | ------ | ------ |
| esp8266_main  | ESP8266          | Controller | No     | ?      |
| esp32_main    | ESP32            | Controller | Yes    | Yes    |
| arduino_ui    | Arduino Pro      | UI         | Yes    | Yes    |
| arduino_test  | Arduino Pro      | Both       | Yes    | No     |
| leonardo_main | Arduino Leonardo | Controller | Yes    | Yes    |
| leonardo_test | Arduino Leonardo | Both       | Yes    | No     |

The "unusable" test targets were only used for easier development.
They do not have enough GPIOs for all functionality!

ESP8266 should work in theory, as long as the board has enough GPIOs, but I have not tested it.

See 'include/config_pins.h' for the GPIO number definitions for the different configurations.
Some other options can be set in 'include/config.h'.

When an ESP is used, a webinterface and OTA updater will be included as well.
For the ESP32, you can upload a new firmware using a webbrowser.
Simply use the link on the main page of the web ui, then upload the '.pio/build/esp32_main/firmware.bin' file from the project directory after building.

## License

    Copyright (c) 2021 Thomas Buck <thomas@xythobuz.de>

    Giess-o-mat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Giess-o-mat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Giess-o-mat.  If not, see <https://www.gnu.org/licenses/>.
