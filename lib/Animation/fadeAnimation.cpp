#include <Arduino.h>

#include "fadeAnimation.h"

int* FadeAnimation::getRandomInitialDelays(FadePattern fadeType, int fadeMultiplier) {
  static int initialDelays[NUM_TUBES];
  int* randomTubeOrder = (fadeType == FadeRandom) ? getRandomTubeOrder() : NULL;

  for (int i = 0; i < NUM_TUBES; i++) {
    switch (fadeType) {
      case FadeLeft:
        initialDelays[i] = i * fadeMultiplier; break;
      case FadeRight:
        initialDelays[i] =  (NUM_TUBES - i - 1) * fadeMultiplier; break;
      case FadeRandom:
        initialDelays[i] =  randomTubeOrder[i] * fadeMultiplier; break;
    }
  }

  return initialDelays;
}

int* FadeAnimation::getRandomTubeOrder() {
  static int randomTubeOrder[NUM_TUBES];
  
  for (int i = 0; i < NUM_TUBES; i++) {
    randomTubeOrder[i] = i;
  }
  for (int i = 0; i < NUM_TUBES; i++) {
    int randomIndex = random(0, NUM_TUBES);
    int temp = randomTubeOrder[i];
    randomTubeOrder[i] = randomTubeOrder[randomIndex];
    randomTubeOrder[randomIndex] = temp;
  }

  return randomTubeOrder;
}

void FadeAnimation::setNextProgression() {
  Serial.printf("FadeProgression:%d->", _fadeProgression);
  _fadeProgression = (FadeProgression)((int)_fadeProgression + 1);
  Serial.println(_fadeProgression);

  int randomDelay = (_fadeProgression == FadeIn) ? random(1000) : 0;
  switch (_fadeProgression) {
    case FadeOut:
      setRandomFadePattern(0);
      break;
    case FadeIn:
     // TODO: configuratible max brightness (daytime/nightnime, I2C light sensor, etc)
      setRandomFadePattern(_maxBrightness, randomDelay);
      break;
    case FadeComplete:
      setDuration(-1);
      break;
  }
}

void FadeAnimation::setRandomFadePattern(int targetBrightness, int initialDelay) {
  int durationMs = random(200, 2000);
  int initialDelayMultiplier = random(0, 1000);
  int* initialDelays = getRandomInitialDelays((FadePattern)random(FadePatternCount), initialDelayMultiplier);

  for (int i = 0; i < NUM_TUBES; i++) {
    setTubeFade(i, targetBrightness, durationMs, initialDelay + initialDelays[i]);
  }
}

void FadeAnimation::setTubeFade(int tubeIndex, int targetBrightness, int durationMs, int initialDelay = 0) {
  _tubeFadeState[tubeIndex].targetBrightness = targetBrightness;
  _tubeFadeState[tubeIndex].stepDelay = initialDelay;
  _tubeFadeState[tubeIndex].stepsLeft = max(1,  (int) ceil(durationMs / FadeStepDelay));
}

void FadeAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("FadeAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(30000);
  _fadeProgression = FadeStart;
  setNextProgression();
}

TickResult FadeAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool update = false;
  for (int i = 0; i < NUM_TUBES; i++) {
    if (_tubeFadeState[i].stepsLeft == 0) {
      continue;
    }

    _tubeFadeState[i].stepDelay--;

    if (_tubeFadeState[i].stepDelay < 0) {
      tubes[i].Brightness += ((int)_tubeFadeState[i].targetBrightness - (int)tubes[i].Brightness) / (int)_tubeFadeState[i].stepsLeft;
      _tubeFadeState[i].stepsLeft--;
      
      if (_tubeFadeState[i].stepsLeft > 0) {
        _tubeFadeState[i].stepDelay = FadeStepDelay;
      }

      update = true;
    }
  }

  if (update) {
    bool shouldProgress = true;
    for (int i = 0; i < NUM_TUBES; i++) {
      shouldProgress &= _tubeFadeState[i].stepsLeft == 0;
    }

    if (shouldProgress) {
      setNextProgression();
    }
  }

  return {false, update};
}
