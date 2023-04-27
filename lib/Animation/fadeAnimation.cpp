#include <Arduino.h>

#include "fadeAnimation.h"

void FadeAnimation::setFade(Tube tubes[NUM_TUBES], int targetBrightness) {

  Serial.println(("FadeAnimatin::SetFade");)
  Serial.printf("B:%3d|%3d|%3d|%3d|%3d|%3d\n", Tubes[5].Brightness, Tubes[4].Brightness, Tubes[3].Brightness, Tubes[2].Brightness, Tubes[1].Brightness, Tubes[0].Brightness);
  Serial.printf("T:%d", targetBrightness);

  int brightnessStep = 8;
  for (int i = 0; i < NUM_TUBES; i++) {
    int brightnessDelta = targetBrightness - tubes[i].Brightness;
    int brightnessStepCount = ceil(abs(brightnessDelta) / brightnessStep);
    int fadeDurationMs = i * 1024;
    int fadeStepDelayMs = fadeDurationMs / brightnessStepCount;

    _tubeFadeState[i].targetBrightness = 0;
    _tubeFadeState[i].step = brightnessDelta > 0 ? brightnessStep : -brightnessStep;
    _tubeFadeState[i].delayReset = fadeStepDelayMs;
    _tubeFadeState[i].delay = fadeStepDelayMs; // add initial delay for offset
    _tubeFadeState[i].isComplete = false;

    Serial.printf("%d|TB%d|S%d|D%d\n",
      i, _tubeFadeState[i].targetBrightness, _tubeFadeState[i].step, _tubeFadeState[i].delay);
    // Serial.printf("trBrt:%d\n", _tubeFadeState[i].targetBrightness);
    // Serial.printf("step:%d\n", _tubeFadeState[i].step);
    // Serial.printf("delayReset:%d\n", _tubeFadeState[i].delayReset);
    // Serial.printf("delay:%d\n", _tubeFadeState[i].delay);
  }
}

void FadeAnimation::initialize(Tube tubes[NUM_TUBES]) {
  Animation::setDuration(20000);
  setFade(tubes, 0);
  _fadeProgression = FadeOut;
}

TickResult FadeAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);
  bool update = false;
  for (int i = 0; i < NUM_TUBES; i++) {
    if (_tubeFadeState[i].isComplete) {
      continue;
    }

    _tubeFadeState[i].delay--;

    if (_tubeFadeState[i].delay < 0) {
      tubes[i].Brightness += _tubeFadeState[i].step; // todo: overflow/underflow

      // todo: helper/util function
      if (tubes[i].Brightness > PWM_MAX) {
        tubes[i].Brightness = PWM_MAX;
      }
      else if (tubes[i].Brightness < PWM_MIN) {
        tubes[i].Brightness = PWM_MIN;
      }

      if (tubes[i].Brightness == _tubeFadeState[i].targetBrightness) {
        Serial.printf("Tube %d fade is complete\n", i);
        _tubeFadeState[i].isComplete = true;
      }
      else {
        _tubeFadeState[i].delay = _tubeFadeState[i].delayReset;
      }

      update = true;
      _tubeFadeState[i].delay = _tubeFadeState[i].delayReset;
    }

    if (update) {
      bool shouldProgress = true;
      for (int i = 0; i < NUM_TUBES; i++) {
        shouldProgress &= _tubeFadeState[i].isComplete;
      }

      if (shouldProgress) {
        Serial.printf("FadeProgression:%d->", _fadeProgression);
        _fadeProgression = (FadeProgression)((int)_fadeProgression + 1);
        Serial.printLN(_fadeProgression);

        if (_fadeProgression == FadeIn) {
          setFade(tubes, PWM_MAX);
        }
        else if (_fadeProgression == FadeComplete) {
          setDuration(-1);
        }
      }
    }
  }
  return {false,update};
}
