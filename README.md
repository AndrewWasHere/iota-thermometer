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
Create `secrets.h` and `secrets.cpp` in the sketch. Copy and paste the
following code into the appropriate files, and replace necessary values.

secrets.h:
```cpp
#ifndef IOTA_SECRETS_H
#define IOTA_SECRETS_H

struct Secrets {
    char const * ssid;
    char const * password;
    char const * iota_id;
    char const * iota_url;
};

extern Secrets secrets;
#endif
```

secrets.cpp:
```cpp
#include "secrets.h"
Secrets secrets = { "<ssid>", "<password>", "<iota_id>", "http://<iota_url>" };
```

