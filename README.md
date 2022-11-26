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

There is also an optional Telegram bot integration.

## Optional Networking Interfaces

There are both a Telegram bot implementation, as well as an MQTT client implemented in Giess-o-mat.
Telegram allows control even from the Internet, but unfortunately polling for new messages is very time consuming.
So I would not recommend enabling it if you also want to use the I2C UI.

Register a new bot with the Telegram botfather and put the token into wifi.h as TELEGRAM_TOKEN.
Compile and run the project, then send a message to the bot.
Look for the chat ID in the log and put it into wifi.h in TRUSTED_IDS.

    echo '#define TELEGRAM_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"' >> include/wifi.h
    echo '#define TRUSTED_IDS { "1234", "5678" }' >> include/wifi.h

MQTT is far less resource intensive, but also does not provide a bridge to the Internet.
Enable it like this.
The last two parameters are optional.

    echo '#define MQTT_HOST "192.168.0.100"' >> include/wifi.h
    echo '#define MQTT_PORT 1883' >> include/wifi.h
    echo '#define MQTT_USER "USERNAME"' >> include/wifi.h
    echo '#define MQTT_PASS "PASSWORD"' >> include/wifi.h

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

Writing events to InfluxDB is also supported.
Configure the server and database in 'include/config.h'.
It will store the valve-open duration (in seconds) with the following measurement types:

    fertilizer
    plant
    calibrated_filling
    calibrated_watering

Fertilizer shows each time a fertilizer pump has been running.
Plant shows when multiple plants have been watered at the same time.
Additionally, calibrated_filling can be used to determine the inlet flowrate and calibrated_watering can be used to determine each outlet flowrate, as long as the tank size is known.

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
