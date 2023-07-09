#include "history.h"

char * const trend(Trend t) {
  if (t == TREND_FALLING) return "falling";
  if (t == TREND_STEADY) return "steady";
  if (t == TREND_RISING) return "rising";
  else return "unknown trend";
}

History::History() : size(0) {}

void History::push(float value) {
  if (size == 3) {
    // History is complete. Shift and store.
    for(std::size_t idx = 0; idx < size - 1; ++idx) {
      values[idx] = values[idx + 1];
    }
    values[size - 1] = value;
  }
  else {
    // History is incomplete. Just store.
    values[size++] = value;
  }
}

Trend History::trend() {
  float const large_threshold = 1.0;
  float const threshold = 0.5;
  float short_delta = size > 1 ? (values[size - 1] - values[size - 2]) : 0.0;
  float long_delta = values[2] - values[0];

  if (size < 3) {
    if (short_delta > large_threshold) return TREND_RISING;
    if (short_delta < -large_threshold) return TREND_FALLING;
    return TREND_STEADY;
  }
  
  if (
    short_delta > large_threshold
    || (
      values[0] < values[1]
      && values[1] < values[2]
      && long_delta >= threshold
    )
  ) {
    return TREND_RISING;
  }

  if (
    short_delta < -large_threshold
    || (
      values[0] > values[1]
      && values[1] > values[2]
      && long_delta <= -threshold
    )
  ) {
    return TREND_FALLING;
  }

  return TREND_STEADY;
}
