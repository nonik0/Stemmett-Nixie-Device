#include <Arduino.h>

#include "cylonScanAnimation.h"

void CylonScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("CylonScanAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness);

  _activePhaseDuration = random(10000, 20000);
  _isActivePhase = true;
  _eyeDelay = 0;
  _eyeShiftDelay = random(300,1500) / NUM_TUBES;
  _eyeFadeDuration = random(_eyeShiftDelay, _eyeShiftDelay * 3);
  _eyeIndex = random(2) == 0 ? 0 : NUM_TUBES - 1;
  _eyeDirection = _eyeIndex == 0 ? Left : Right; 
  _slotDelay = random(10,40);
}


TickResult CylonScanAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activePhaseDuration--;
  _eyeDelay--;

  // end active phase on end tube
  if (_activePhaseDuration < 0 && _isActivePhase && (_eyeIndex == 0 || _eyeIndex == NUM_TUBES-1)) {
    _isActivePhase = false;
  }

  if (_eyeDelay < 0) {
    _slotHelper.disableSlot(_eyeIndex);

    // fade out only if active phase, otherwise leave at max brightness
    if (_isActivePhase) {
      _fadeHelper.setTubeFade(_eyeIndex, 0, _eyeFadeDuration);
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

    // shift eye location and change direction if at end
    _eyeIndex += _eyeDirection;
    if (_eyeIndex == 0) {
      _eyeDirection = Left;
    }
    if (_eyeIndex == NUM_TUBES - 1) {
      _eyeDirection = Right;
    }


    // enable slot and set tube to max brightness
    _slotHelper.enableSlot(_eyeIndex, _slotDelay);
    _fadeHelper.setTubeFade(_eyeIndex, _maxBrightness, 0);   
    _eyeDelay = _eyeShiftDelay;
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}