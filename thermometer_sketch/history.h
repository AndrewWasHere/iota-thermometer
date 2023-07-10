#ifndef IOTA_HISTORY
#define IOTA_HISTORY

#include <cstddef>

struct History {
  History();
  void push(float value);
  float latest();
  float previous();
  float first();
  bool full();
  bool empty();

  size_t n_values;
  float values[3];
};

#endif