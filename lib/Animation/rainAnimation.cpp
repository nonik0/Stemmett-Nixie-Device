#include <Arduino.h>

#include "rainAnimation.h"

void RainAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("RainAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(25000);

  // TODO: init: bubbles always glow to max brightness, or random brightness up to max
  // TODO: multiplier to create "fast" and "slow" overall animation
  // TODO: slot animation during fade..?

  //_activeBubbleCount = 0;
  _activePhaseDuration = random(10000,20000);
  _newRaindropCooldown = 0;
  _baseCooldown = random(5,25);
  for (int i = 0; i < NUM_TUBES; i++) {
    _raindrops[i].isActive = true;
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

    // when not active, an active slot randomly deactivates to prevent all 6 in a row
    _raindrops[tubeIndex].isActive = _raindrops[tubeIndex].isActive && (isActivePhase ? true : random(6) != 0);
    if (_raindrops[tubeIndex].isActive) {
      _raindrops[tubeIndex].fadeDuration = random(100, 600);;
      _raindrops[tubeIndex].initialBrightness = random(0.7 * _maxBrightness, _maxBrightness);
      
      _fadeHelper.setTubeFade(tubeIndex, _raindrops[tubeIndex].initialBrightness, 0);
      _slotHelper.setRandomCathode(tubeIndex);
    }
    else {
      _fadeHelper.setTubeFade(tubeIndex, _maxBrightness, 0);
      _slotHelper.setPrimaryCathode(tubeIndex);
    }

    _newRaindropCooldown = random(_baseCooldown, _baseCooldown*3);
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}