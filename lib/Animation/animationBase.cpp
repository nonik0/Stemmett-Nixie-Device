#include "animationBase.h"

void Animation::setDuration(int durationMs) { _durationMs = durationMs; }

void Animation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  _maxBrightness = maxBrightness;
}

TickResult Animation::handleTick(Tube tubes[NUM_TUBES]) {
  _durationMs--;
  return {false, false};
}

bool Animation::isComplete() {
  return _durationMs < 0;
}
