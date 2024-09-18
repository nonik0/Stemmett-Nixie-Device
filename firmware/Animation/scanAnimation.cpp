#include <Arduino.h>

#include "scanAnimation.h"

void ScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("ScanAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness, speedFactor);

  _isScanComplete = true;
  _isSlotScan = random(4) > 0;
  _scanDelay = 0;
  _scanInactiveBrightness = _maxBrightness / random(4,8);
  _scanIndex = random(2) == 0 ? -1 : NUM_TUBES;

  int minPauseDelay = 200 * (1 - speedFactor); // slowest: 200, fastest: 0
  int maxPauseDelay = 500 + 500 * (1 - speedFactor); // slowest: 1000, fastest: 500
  _scanPauseDelay = random(minPauseDelay, maxPauseDelay);

  int minShiftDelay = 100 + 200 * (1 - speedFactor); // slowest: 300, fastest: 100
  int maxShiftDelay = 500 + 500 * (1 - speedFactor); // slowest: 1000, fastest: 500
  _scanShiftDelay = random(minShiftDelay, maxShiftDelay) / NUM_TUBES;
  _scanDirection = _scanIndex <= 0 ? Left : Right;
  
  int minScans = 5 + 5 * speedFactor;
  int maxScans = 10 + 10 * speedFactor;
  _scansLeft = random(minScans, maxScans);
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