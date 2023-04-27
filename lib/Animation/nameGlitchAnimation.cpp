#include "nameGlitchAnimation.h"

void NameGlitchAnimation::initialize(Tube tubes[NUM_TUBES]) {
  Animation::setDuration(2000);
  _firstTick = true;
}

TickResult NameGlitchAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  if (_firstTick) {
    for (int i = 0; i < NUM_TUBES; i++) {
      tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
      tubes[i].Brightness = 150;
    }
    return {true,true};
  }

  return {false,false};
}
