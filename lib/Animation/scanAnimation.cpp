#include <Arduino.h>

#include "scanAnimation.h"

void ScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("ScanAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness);

  _isScanComplete = true;
  _isSlotScan = true;
  _scanDelay = 0;
  _scanInactiveBrightness = _maxBrightness / random(4,8);
  _scanIndex = random(2) == 0 ? -1 : NUM_TUBES;
  _scanPauseDelay = random(0,500);
  _scanShiftDelay = random(150,600) / NUM_TUBES;
  _scanDirection = _scanIndex <= 0 ? Left : Right;
  _scansLeft = random(15,30);
}

TickResult ScanAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _scanDelay--;

  if (_scanDelay < 0) {
    if (!_isScanComplete) {
      if (_scansLeft > 0) {
        _slotHelper.setRandomCathode(_scanIndex);
        _fadeHelper.setTubeBrightness(_scanIndex, _scanInactiveBrightness);
      }
      else {
        _slotHelper.setPrimaryCathode(_scanIndex);
        _fadeHelper.setTubeBrightness(_scanIndex, _maxBrightness);
      }
    }

    _scanIndex += _scanDirection;
    _isScanComplete = _scanDirection == Left ? _scanIndex >= NUM_TUBES : _scanIndex < 0;

    if (_isScanComplete) {
      if (_scansLeft == 0) {
        setComplete();
        return {false, false};
      }

      _scansLeft--;
      _scanIndex = _scanDirection == Left ? 0 + Right : NUM_TUBES - 1 + Left; // start "off screen"
      _scanDelay = _scanPauseDelay;
    }
    else {
      if (_isSlotScan) {
        _slotHelper.enableCycling(_scanIndex, 5);
      }
      else {
        _slotHelper.setRandomCathode(_scanIndex);
      }
      
      _fadeHelper.setTubeBrightness(_scanIndex, _maxBrightness);
      _scanDelay = _scanShiftDelay;
    }
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}