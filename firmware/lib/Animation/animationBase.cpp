#include <Arduino.h>

#include "animationBase.h"

void Animation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  _maxBrightness = maxBrightness;
}

TickResult Animation::handleTick(Tube tubes[NUM_TUBES]) {
  _durationMs--;
  return {false, false};
}

bool Animation::isComplete() { return _durationMs < 0; }

void Animation::setComplete() {
  Serial.println("Animation::setComplete");
  _durationMs = -1;
}

void Animation::setDuration(int durationMs) { _durationMs = durationMs; }