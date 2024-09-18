#include <Arduino.h>

#include "ghostAnimation.h"

void GhostAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("GhostAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(25000);

  // TODO: init: ghosts always glow to max brightness, or random brightness up to max
  // TODO: multiplier to create "fast" and "slow" overall animation
  // TODO: slot animation during fade..?

  _activeGhostCount = 0;
  _activePhaseDuration = random(10000,20000);
  _newGhostCooldown = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    _ghost[i].isActive = false;
  }
}

TickResult GhostAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool update = _fadeHelper.handleTick(tubes);

  _activePhaseDuration--;
  _newGhostCooldown--;

  bool isActivePhase = _activePhaseDuration > 0;
  int activeGhostCountTarget = 5;

  // check if complete if not in active phase
  if (!isActivePhase) {
    int tubeCompletedCount = 0;
    for (int i = 0; i < NUM_TUBES; i++) {
      if (tubes[i].Brightness == _maxBrightness) {
        tubeCompletedCount++;
      }
    }

    if (tubeCompletedCount < NUM_TUBES) {
      activeGhostCountTarget = NUM_TUBES - tubeCompletedCount;
    }
    else {
      setComplete();
      return {false, update};
    }
  }

  // handle ghost activation and fade in 
  if (_newGhostCooldown < 0 && _activeGhostCount < activeGhostCountTarget)
  {
    int tubeIndex;
    do {
      tubeIndex = random(NUM_TUBES);
    } while (_ghost[tubeIndex].isActive || (!isActivePhase && tubes[tubeIndex].Brightness == _maxBrightness));

    int targetBrightness = isActivePhase ? random(20, _maxBrightness) : _maxBrightness;
    int minFadeDuration = 200 + 400 *(1 - _speedFactor); // slowest: 600, fastest: 200
    int maxFadeDuration = 800 + 1200 *(1 - _speedFactor); // slowest: 2000, fastest: 800
    int fadeDuration = random(minFadeDuration, maxFadeDuration);

    _ghost[tubeIndex].isActive = true;
    _ghost[tubeIndex].fadeDuration = fadeDuration;
    _ghost[tubeIndex].targetBrightnessLow = random(0, 30);
    _ghost[tubeIndex].targetBrightnessHigh = targetBrightness;
    
    _fadeHelper.setTubeFade(tubeIndex, targetBrightness, fadeDuration);

    _activeGhostCount++;

    int minGhostCooldown = 60 + 140 * (1 - _speedFactor); // slowest: 200, fastest: 60
    int maxGhostCooldown = 800 + 1200 * (1 - _speedFactor); // slowest: 2000, fastest: 800
    _newGhostCooldown = random(minGhostCooldown, maxGhostCooldown); // TODO: randomize
  }

  // handle ghost fade down and deactivation
  for (int i = 0; i < NUM_TUBES; i++) {
    if (_ghost[i].isActive && _fadeHelper.isComplete(i)) {
      if (tubes[i].Brightness == _ghost[i].targetBrightnessHigh) {
        if (isActivePhase) {
          _fadeHelper.setTubeFade(i, _ghost[i].targetBrightnessLow, _ghost[i].fadeDuration);
        }
        else {
          _ghost[i].isActive = false;
          _activeGhostCount--;
        }
      }
      else {
        _ghost[i].isActive = false;
        _activeGhostCount--;
      }
    }
  }

  return {false, update};
}