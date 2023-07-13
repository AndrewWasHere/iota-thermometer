#include "trend.h"

/*
  Determine current Trend based on `h` and `threshold`.
*/
Trend trend(History & h, float const threshold) {
  if (!h.full() && !h.empty()) {
    float delta = h.latest() - h.previous();
    if (delta >= threshold) {
      return TREND_RISING;
    }
    if (delta <= -threshold) {
      return TREND_FALLING;
    }
    return TREND_STEADY;
  }

  if (h.full()) {
    float delta = h.latest() - h.first();

    if (
      h.first() < h.previous() && h.previous() < h.latest() && 
      delta >= threshold
    ) {
      return TREND_RISING;
    }
    if (
      h.first() > h.previous() && h.previous() > h.latest() && 
      delta <= -threshold
    ) {
      return TREND_FALLING;
    }
    return TREND_STEADY;
  }

  return TREND_STEADY;
}

/*
  Human-readable Trend.
*/
char * const trend_str(Trend const t) {
  if (t == TREND_FALLING) return "falling";
  if (t == TREND_STEADY) return "steady";
  if (t == TREND_RISING) return "rising";
  else return "unknown trend";
}

