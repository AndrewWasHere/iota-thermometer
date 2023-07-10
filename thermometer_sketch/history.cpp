#include "history.h"

#define LATEST 0
#define PREVIOUS 1
#define FIRST 2

History::History() : n_values(0) {}

void History::push(float value) {
  values[FIRST] = values[PREVIOUS];
  values[PREVIOUS] = values[LATEST];
  values[LATEST] = value;

  if (n_values < 3) ++n_values;
}

float History::latest() {
  return values[LATEST];
}

float History::previous() {
  return values[PREVIOUS];
}

float History::first() {
  return values[FIRST];
}

bool History::full() {
  return n_values >= 3;
}

bool History::empty() {
  return n_values == 0;
}