#include <Arduino.h>

#include "bubbleAnimation.h"

void BubbleAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("BubbleAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(55000);

  _activePhaseDuration = random(10000,20000);
  _newBubbleCooldown = 0;
  _baseCooldown = random(20,100);
  for (int i = 0; i < NUM_TUBES; i++) {
    _bubble[i].isActive = true;
  }
}

TickResult BubbleAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activePhaseDuration--;
  _newBubbleCooldown--;

  bool isActivePhase = _activePhaseDuration > 0;

  // check if complete if not in active phase
  int tubeCompletedCount = 0;
  int tubeInactiveCount = 0;
  if (!isActivePhase) {
    for (int i = 0; i < NUM_TUBES; i++) {
      if (!_bubble[i].isActive) {
        tubeInactiveCount++;
      
        if (tubes[i].Brightness == _maxBrightness) {
          tubeCompletedCount++;
        }
      }
    }

    if (tubeCompletedCount == NUM_TUBES) {
      setComplete();
      return {false, false};
    }
  }

  for (int i = 0; i < NUM_TUBES; i++) {
    if (tubes[i].Brightness == 0) {
      //if (!isActivePhase) printf("fade in bubble %d\n", i);
      _fadeHelper.setTubeFade(i, _maxBrightness, _bubble[i].fadeDuration);
      _slotHelper.setRandomCathode(i);
    }
  }  

  if (_newBubbleCooldown < 0 && tubeInactiveCount < NUM_TUBES)
  {
    int tubeIndex;
    if (isActivePhase) {
      tubeIndex = random(NUM_TUBES);
    }
    else {
      do {
        tubeIndex = random(NUM_TUBES);
      } while (!_bubble[tubeIndex].isActive);
    }

    _bubble[tubeIndex].isActive = isActivePhase
      ? _bubble[tubeIndex].isActive || random(6) == 0
      : _bubble[tubeIndex].isActive && random(NUM_TUBES - tubeCompletedCount) != 0;

    if (_bubble[tubeIndex].isActive) {
      //if (!isActivePhase) Serial.printf("new bubble %d\n", tubeIndex);
      _bubble[tubeIndex].fadeDuration = random(_baseCooldown*5, _baseCooldown*10);
      _fadeHelper.setTubeFade(tubeIndex, 0, 0);
    }
    else {
      _slotHelper.setPrimaryCathode(tubeIndex);
    }

    _newBubbleCooldown = random(_baseCooldown, _baseCooldown*3);
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}