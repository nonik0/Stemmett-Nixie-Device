#include <Arduino.h>

#include "animationBase.h"

void Animation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  _maxBrightness = maxBrightness;
  _speedFactor = speedFactor;
}

TickResult Animation::handleTick(Tube tubes[NUM_TUBES]) {
  _durationMs--;
  return {false, false};
}

bool Animation::isComplete() { return _durationMs < 0; }

void Animation::setComplete() {
  log_d("Animation::setComplete");
  _durationMs = -1;
}

void Animation::setBrightness(int brightness) { _maxBrightness = brightness; }

void Animation::setDuration(int durationMs) { _durationMs = durationMs; }\

void Animation::setSpeed(float speedFactor) { 
  if (speedFactor < 0) speedFactor = 0;
  if (speedFactor > 1) speedFactor = 1;

  _speedFactor = speedFactor;
}