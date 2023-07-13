#ifndef IOTA_TREND_H
#define IOTA_TREND_H

#include "history.h"

enum Trend { TREND_FALLING, TREND_STEADY, TREND_RISING };
Trend trend(History & h, float const threshold);
char * const trend_str(Trend const t);

#endif