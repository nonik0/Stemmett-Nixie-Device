#include <Arduino.h>

#include "fadeHelper.h"

FadeHelper::FadeHelper() {
  for (int i = 0; i < NUM_TUBES; i++) {
    _tubeFadeState[i].targetBrightness = 0;
    _tubeFadeState[i].stepDelay = 0;
    _tubeFadeState[i].stepsLeft = 0;
  }
}

bool FadeHelper::isComplete(int tubeIndex) {
  if (tubeIndex == -1) {
    bool allComplete = true;
    for (int i = 0; i < NUM_TUBES; i++) {
      allComplete &= _tubeFadeState[i].stepsLeft == 0;
    }
    return allComplete;
  }

  return _tubeFadeState[tubeIndex].stepsLeft == 0;
}

void FadeHelper::setTubeBrightness(int tubeIndex, int targetBrightness, int initialDelay) {
  _tubeFadeState[tubeIndex].targetBrightness = targetBrightness;
  _tubeFadeState[tubeIndex].stepDelay = initialDelay;
  _tubeFadeState[tubeIndex].stepsLeft = 0;
}

void FadeHelper::setTubeFade(int tubeIndex, int targetBrightness, int durationMs, int initialDelay) {
  _tubeFadeState[tubeIndex].targetBrightness = targetBrightness;
  _tubeFadeState[tubeIndex].stepDelay = initialDelay;
  _tubeFadeState[tubeIndex].stepsLeft = max(1,  (int) ceil(durationMs / FadeStepDelay));
}

bool FadeHelper::handleTick(Tube tubes[NUM_TUBES]) {
  bool update = false;

  for (int i = 0; i < NUM_TUBES; i++) {
    if (_tubeFadeState[i].stepsLeft > 0) {
      _tubeFadeState[i].stepDelay--;

      if (_tubeFadeState[i].stepDelay < 0) {
        tubes[i].Brightness += ((int)_tubeFadeState[i].targetBrightness - (int)tubes[i].Brightness) / (int)_tubeFadeState[i].stepsLeft;
        _tubeFadeState[i].stepsLeft--;
        _tubeFadeState[i].stepDelay = FadeStepDelay;
        update = true;
      }
    }
  }

  return update;
}