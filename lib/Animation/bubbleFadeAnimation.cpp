#include <Arduino.h>

#include "bubbleFadeAnimation.h"

void BubbleFadeAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("BubbleFadeAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(25000);

  // TODO: init: bubbles always glow to max brightness, or random brightness up to max
  // TODO: multiplier to create "fast" and "slow" overall animation

  _activeBubbleCount = 0;
  _activeBubbleDuration = random(10000,20000);
  _newBubbleCooldown = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    _bubbles[i].isActive = false; // will do initial fade out

    // will all fade out at the same time
    // _bubbles[i].fadeDuration = random(300, 1000);
    // _bubbles[i].targetBrightness = 0;
  }
}

TickResult BubbleFadeAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activeBubbleDuration--;
  _newBubbleCooldown--;

  bool isActivePhase = _activeBubbleDuration > 0;

  bool update = _fadeHelper.handleTick(tubes);

  for (int i = 0; i < NUM_TUBES; i++) {
    if (_bubbles[i].isActive && _fadeHelper.isComplete(i)) {
      if (tubes[i].Brightness == _bubbles[i].targetBrightness) {
        if (sisActivePhase) {
          _fadeHelper.setTubeFade(i, 0, _bubbles[i].fadeDuration);
        }
      }
      else {
        _bubbles[i].isActive = false;
        _activeBubbleCount--;
      }
    }
  }

  
  int activeBubbleCountTarget = isActivePhase ? 5 6;

  if (_newBubbleCooldown < 0 && _activeBubbleCount < activeBubbleCountTarget)
  {
    //Serial.printf("BubbleFadeAnimation::handleTick: _bubbleCooldown: %d, _activeBubbleCount: %d\n", _bubbleCooldown, _activeBubbleCount);
    // todo: better way
    int tubeIndex;
    do {
      tubeIndex = random(NUM_TUBES);
    } while (_bubbles[tubeIndex].isActive);

    int targetBrightness = isActivePhase ? random(20, _maxBrightness) : _maxBrightness;
    int fadeDuration = random(300, 1000);

    _bubbles[tubeIndex].isActive = true;
    _bubbles[tubeIndex].fadeDuration = fadeDuration; // TODO: test
    _bubbles[tubeIndex].targetBrightness = targetBrightness;
    
    _fadeHelper.setTubeFade(tubeIndex, targetBrightness, fadeDuration);

    _activeBubbleCount++;
    _newBubbleCooldown = random(100,1000); // TODO: randomize
  }

  if (isActivePhaseComplete) {
    if (_fadeHelper.isComplete()) {
      Animation::setDuration(-1);
    }
  }

  return {false, update};
}