#include <Arduino.h>

#include "cylonScanAnimation.h"

void CylonScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("CylonScanAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness, speedFactor);

  _activePhaseDuration = random(10000, 20000);
  _isActivePhase = true;
  _eyeDelay = 0;
  _eyeShiftDelay = random(300 + 1000*(1-speedFactor), 1500 + 200*(1-speedFactor)) / NUM_TUBES;
  _eyeFadeDuration = random(_eyeShiftDelay, _eyeShiftDelay * 3);
  _eyeIndex = random(2) == 0 ? 0 : NUM_TUBES - 1;
  _eyeDirection = _eyeIndex == 0 ? Left : Right; 
  _slotDelay = random(10 + 30*(1-speedFactor), 40 + 20*(1-speedFactor));

  log_d("eyeShiftDelay: %d", _eyeShiftDelay);
  log_d("eyeFadeDuration: %d", _eyeFadeDuration);
  log_d("slotDelay: %d", _slotDelay);
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
    _slotHelper.disableCycling(_eyeIndex);

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
        setComplete();
        return {true, true};
      }
    }

    // shift eye location and change direction if at end
    _eyeIndex += _eyeDirection;
    if (_eyeIndex == 0) {
      _eyeDirection = Left;
    }
    else if (_eyeIndex == NUM_TUBES - 1) {
      _eyeDirection = Right;
    }

    // enable slot and set tube to max brightness
    _slotHelper.enableCycling(_eyeIndex, _slotDelay);
    _fadeHelper.setTubeBrightness(_eyeIndex, _maxBrightness);   
    _eyeDelay = _eyeShiftDelay;
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}