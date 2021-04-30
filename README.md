# Giess-o-mat

For more please also [take a look at Giess-o-mat on my website](https://www.xythobuz.de/giessomat.html).

## Quick Start

If you don't already have it, get [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html).

Before building for an ESP32 or ESP8266 target, execute the following commands, replacing "..." with the values of your network, so you can connect to WiFi.

    echo '#define WIFI_SSID "..."' > include/wifi.h
    echo '#define WIFI_PW "..."' >> include/wifi.h

Then simply run the build for all supported configurations with platformio:

    pio run

You can of course also use pio to flash your targets.

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
