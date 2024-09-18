#include <Arduino.h>

#include "rainAnimation.h"

void RainAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("RainAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(25000);

  _activePhaseDuration = random(10000,20000);
  _newRaindropCooldown = 0;

  int minBaseCooldown = 5 + 15 * (1 - speedFactor); // slowest: 20, fastest: 5
  int maxBaseCooldown = 25 + 175 * (1 - speedFactor); // slowest: 200, fastest: 25
  _baseCooldown = random(minBaseCooldown, maxBaseCooldown);
  for (int i = 0; i < NUM_TUBES; i++) {
    _raindrops[i].isActive = false;
  }
}

bool once = false;
TickResult RainAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activePhaseDuration--;
  _newRaindropCooldown--;

  bool isActivePhase = _activePhaseDuration > 0;

  // check if complete if not in active phase
  if (!isActivePhase) {
    int tubeCompletedCount = 0;
    for (int i = 0; i < NUM_TUBES; i++) {
      if (!_raindrops[i].isActive) {
        tubeCompletedCount++;
      }
    }

    if (tubeCompletedCount == NUM_TUBES) {
      setComplete();
      return {false, false};
    }
  }

  for (int i = 0; i < NUM_TUBES; i++) {
    if (_raindrops[i].isActive && tubes[i].Brightness == _raindrops[i].initialBrightness) {
      _fadeHelper.setTubeFade(i, 0, _raindrops[i].fadeDuration);
    }
  }  

  if (_newRaindropCooldown < 0)
  {
    int tubeIndex = random(NUM_TUBES);

    _raindrops[tubeIndex].isActive = isActivePhase
      ? _raindrops[tubeIndex].isActive || random(6) == 0
      : _raindrops[tubeIndex].isActive && random(6) != 0;

    if (_raindrops[tubeIndex].isActive) {
      int minFadeDuration = 100 + 300*(_speedFactor*100.0); // slowest: 400, fastest: 100
      int maxFadeDuration = 600 + 600*(1 - _speedFactor/100.0); // slowest: 1200, fastest: 600
      _raindrops[tubeIndex].fadeDuration = random(minFadeDuration, maxFadeDuration);
      _raindrops[tubeIndex].initialBrightness = random(0.7 * _maxBrightness, _maxBrightness);
      
      _fadeHelper.setTubeBrightness(tubeIndex, _raindrops[tubeIndex].initialBrightness);
      _slotHelper.setRandomCathode(tubeIndex);
    }
    else {
      _fadeHelper.setTubeBrightness(tubeIndex, _maxBrightness);
      _slotHelper.setPrimaryCathode(tubeIndex);
    }

    _newRaindropCooldown = random(_baseCooldown, _baseCooldown*3);
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}