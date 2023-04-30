#include <Arduino.h>

#include "fadeAnimation.h"

void FadeAnimation::setFade(Tube tubes[NUM_TUBES], int targetBrightness) {

  Serial.println("FadeAnimation::SetFade");
  Serial.printf("curBrt:%3d|%3d|%3d|%3d|%3d|%3d\n",
    tubes[5].Brightness,
    tubes[4].Brightness,
    tubes[3].Brightness,
    tubes[2].Brightness,
    tubes[1].Brightness,
    tubes[0].Brightness);
  Serial.printf("tarBrt:%d\n", targetBrightness);
  Serial.flush();

  int brightnessStep = 8;
  for (int i = 0; i < NUM_TUBES; i++) {
    int brightnessDelta = targetBrightness - tubes[i].Brightness;
    int brightnessStepCount = max(1,  (int) ceil(abs(brightnessDelta) / brightnessStep));
    int fadeDurationMs = i * 1024;
    int fadeStepDelayMs = fadeDurationMs / brightnessStepCount;

    _tubeFadeState[i].targetBrightness = targetBrightness;
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
  Serial.println("FadeAnimation::initialize");

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
      // set to target brightness if difference is smaller than step
      if (abs((int)_tubeFadeState[i].targetBrightness - (int)tubes[i].Brightness) < abs(_tubeFadeState[i].step)) {
        tubes[i].Brightness = _tubeFadeState[i].targetBrightness;
      }
      else {
        tubes[i].Brightness += _tubeFadeState[i].step; // todo: overflow/underflow
      }

      // // todo: helper/util function
      // if (tubes[i].Brightness > PWM_MAX) {
      //   tubes[i].Brightness = PWM_MAX;
      // }
      // else if (tubes[i].Brightness < PWM_MIN) {
      //   tubes[i].Brightness = PWM_MIN;
      // }

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
        Serial.println(_fadeProgression);

        if (_fadeProgression == FadeIn) {
          setFade(tubes, 150);
        }
        else if (_fadeProgression == FadeComplete) {
          setDuration(-1);
        }
      }
    }
  }
  return {false, update};
}
