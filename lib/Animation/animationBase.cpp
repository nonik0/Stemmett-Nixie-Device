#include "animationBase.h"

void Animation::setDuration(int durationMs) { _durationMs = durationMs; }

TickResult Animation::handleTick(Tube tubes[NUM_TUBES]) {
  _durationMs--;
  return {false, false};
}

bool Animation::isComplete() {
  return _durationMs < 0;
}
