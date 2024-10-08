#include <Arduino.h>

#include "basicFadeAnimation.h"

int* BasicFadeAnimation::getRandomInitialDelays(FadePattern fadeType, int fadeMultiplier) {
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

int* BasicFadeAnimation::getRandomTubeOrder() {
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

void BasicFadeAnimation::setNextProgression() {
  log_d("FadeProgression:%d->", _fadeProgression);
  _fadeProgression = (FadeProgression)((int)_fadeProgression + 1);
  log_d("%d", _fadeProgression);

  int randomDelay = (_fadeProgression == FadeIn) ? random(200 + 800*(_speedFactor/100.0)) : 0;
  switch (_fadeProgression) {
    case FadeOut:
      setRandomFadePattern(0);
      break;
    case FadeIn:
     // TODO: configuratible max brightness (daytime/nightnime, I2C light sensor, etc)
      setRandomFadePattern(_maxBrightness, randomDelay);
      break;
    case FadeComplete:
      setComplete();
      break;
  }
}

void BasicFadeAnimation::setRandomFadePattern(int targetBrightness, int initialDelay) {
  int minDurationMs = 200 + 800 * (1 - _speedFactor); // slowest: 1000, fastest: 200
  int maxDurationMs = 600 + 1400 * (1 - _speedFactor); // slowest: 2000, fastest: 600
  int durationMs = random(minDurationMs, maxDurationMs);
  int initialDelayMultiplier = random(200 + 800 * (1 - _speedFactor)); // slowest: 1000, fastest: 200
  int* initialDelays = getRandomInitialDelays((FadePattern)random(FadePatternCount), initialDelayMultiplier);

  for (int i = 0; i < NUM_TUBES; i++) {
    _fadeHelper.setTubeFade(i, targetBrightness, durationMs, initialDelay + initialDelays[i]);
  }
}

void BasicFadeAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("BasicFadeAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(30000);
  _fadeProgression = FadeStart;
  setNextProgression();
}

TickResult BasicFadeAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool update = _fadeHelper.handleTick(tubes);

  if (update && _fadeHelper.isComplete()) {
      setNextProgression();
  }

  return {false, update};
}