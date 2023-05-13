#include <Arduino.h>

#include "cylonScanANimation.h"

void CylonScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("CylonScanAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness);

  _activePhaseDuration = random(10000, 20000);
  _eyeDelay = 0;
  int eyePeriod = random(600,1800);
  _eyeShiftDelay = eyePeriod / NUM_TUBES;
  _eyeIndex = random(2) == 0 ? 0 : NUM_TUBES - 1;
  _eyeDirection = _eyeIndex == 0 ? Left : Right;  
}

TickResult CylonScanAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activePhaseDuration--;
  _eyeDelay--;

  bool brightnessUpdate = false;
  if (_eyeDelay < 0) {
    // fade out current location of eye and disable slot

    _slotHelper.disableSlot(_eyeIndex);

    if (_activePhaseDuration > 0) {
      _fadeHelper.setTubeFade(_eyeIndex, 0, _eyeShiftDelay * 2);
    }
    else {
      bool allComplete = true;
      for (int i = 0; i < NUM_TUBES; i++) {
        allComplete &= tubes[i].Brightness == _maxBrightness;
      }

      if (allComplete) {
        Animation::setDuration(-1);
        return {true, true};
      }
    }

    // shift eye location
    if (_eyeDirection == Left) {
      _eyeIndex++;
      if (_eyeIndex == NUM_TUBES - 1) {
        _eyeDirection = Right;
      }
    }
    else if (_eyeDirection == Right) {
      _eyeIndex--;
      if (_eyeIndex == 0) {
        _eyeDirection = Left;
      }
    }

    // set eye slot and brightness
    tubes[_eyeIndex].Brightness = _maxBrightness;
    _slotHelper.enableSlot(_eyeIndex, 20);
    _eyeDelay = _eyeShiftDelay;
  }

 brightnessUpdate |= _fadeHelper.handleTick(tubes);
  bool cathodeUpdate = _slotHelper.handleTick(tubes);

  return {brightnessUpdate, cathodeUpdate};
}