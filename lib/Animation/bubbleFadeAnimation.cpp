#include <Arduino.h>

#include "bubbleFadeAnimation.h"

void BubbleFadeAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("BubbleFadeAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(25000);

  // TODO: init: bubbles always glow to max brightness, or random brightness up to max
  // TODO: multiplier to create "fast" and "slow" overall animation
  // TODO: slot animation during fade..?

  _activeBubbleCount = 0;
  _activePhaseDuration = random(10000,20000);
  _newBubbleCooldown = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    _bubbles[i].isActive = false;
  }
}

bool once = false;
TickResult BubbleFadeAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool update = _fadeHelper.handleTick(tubes);

  _activePhaseDuration--;
  _newBubbleCooldown--;

  bool isActivePhase = _activePhaseDuration > 0;
  int activeBubbleCountTarget = 5;

  // check if complete if not in active phase
  if (!isActivePhase) {
    int tubeCompletedCount = 0;
    for (int i = 0; i < NUM_TUBES; i++) {
      if (tubes[i].Brightness == _maxBrightness) {
        tubeCompletedCount++;
      }
    }

    if (tubeCompletedCount < NUM_TUBES) {
      activeBubbleCountTarget = NUM_TUBES - tubeCompletedCount;
    }
    else {
      setComplete();
      return {false, update};
    }
  }

  // handle bubble activation and fade up 
  if (_newBubbleCooldown < 0 && _activeBubbleCount < activeBubbleCountTarget)
  {
    int tubeIndex;
    do {
      tubeIndex = random(NUM_TUBES);
    } while (_bubbles[tubeIndex].isActive || (!isActivePhase && tubes[tubeIndex].Brightness == _maxBrightness));

    int targetBrightness = isActivePhase ? random(20, _maxBrightness) : _maxBrightness;
    int fadeDuration = random(300, 1000);

    _bubbles[tubeIndex].isActive = true;
    _bubbles[tubeIndex].fadeDuration = fadeDuration; // TODO: test
    _bubbles[tubeIndex].targetBrightnessLow = random(0, 30);
    _bubbles[tubeIndex].targetBrightnessHigh = targetBrightness;
    
    _fadeHelper.setTubeFade(tubeIndex, targetBrightness, fadeDuration);

    _activeBubbleCount++;
    _newBubbleCooldown = random(100,1000); // TODO: randomize
  }

  // handle bubble fade down and deactivation
  for (int i = 0; i < NUM_TUBES; i++) {
    if (_bubbles[i].isActive && _fadeHelper.isComplete(i)) {
      if (tubes[i].Brightness == _bubbles[i].targetBrightnessHigh) {
        if (isActivePhase) {
          _fadeHelper.setTubeFade(i, _bubbles[i].targetBrightnessLow, _bubbles[i].fadeDuration);
        }
        else {
          _bubbles[i].isActive = false;
          _activeBubbleCount--;
        }
      }
      else {
        _bubbles[i].isActive = false;
        _activeBubbleCount--;
      }
    }
  }

  return {false, update};
}