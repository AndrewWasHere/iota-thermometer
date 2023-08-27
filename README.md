# Iota-Thermometer

An IoT thermometer that pushes data to an iota server.

## Requirements

Hardware:
* Adafruit ESP32-S2 TFT Feather -- http://adafru.it/5300
* Adafruit PCT2075 STEMMA QT -- http://adafru.it/4369
* STEMMA Cable, e.g. JST SH 4-pin cable 100mm -- http://adafru.it/4210

Arduino libraries:
* Adafruit ST7735 and ST7789 Library
* Adafruit PCT2075

## Configuration
Create `secrets.cpp` in the sketch. Copy and paste the following code into the 
appropriate file, and replace necessary values.

secrets.cpp:
```cpp
#include "secrets.h"

Secrets secrets = { 
    "<ssid>",           // replace with WiFi SSID.
    "<password>",       // replace with WiFi password.
    "<iota_id>",        // replace with unique Iota ID.
    "http://<iota_url>" // replace with Iota URL.
};
```
## License

Distributed under the MIT License. See LICENSE.txt for more information.

