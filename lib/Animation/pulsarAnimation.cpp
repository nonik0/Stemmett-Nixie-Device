#include <Arduino.h>

#include "pulsarAnimation.h"

void PulsarAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("PulsarAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness);

  _pulsarIndex = 1;

  _slotHelper.enableSlot(_pulsarIndex);
}


TickResult PulsarAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _pulsarDelay--;
  _pulsarEjectionDelay--;

  if (_pulsarDelay < 0) {

  }

  if (_pulsarEjectionDelay < 0) {



    _pulsarEjectionDelay = _pulsarEjectionFrequency; // TODO: random or periodic?
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}