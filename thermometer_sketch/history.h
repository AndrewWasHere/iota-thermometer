#ifndef IOTA_HISTORY
#defin IOTA_HISTORY

#include <cstddef>

enum Trend { TREND_FALLING, TREND_STEADY, TREND_RISING };
char * const trend(Trend t);

struct History {
  History();
  void push(float value);
  Trend trend();

  std::size_t size;
  float values[3];
};

#endif