#include <Arduino.h>

#include "randomScanAnimation.h"

void RandomScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  log_i("RandomScanAnimation::initialize");
  Animation::setDuration(10000);
  Animation::initialize(tubes, maxBrightness);
  _activeTube = -1;
  _cathodeDelay = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    _tubeProgress[i] = 0;
    tubes[i].Brightness = 0;
  }
}

TickResult RandomScanAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  TickResult result = {false, false};
  _cathodeDelay--;

  if (_cathodeDelay < 0) {
    // set current cathode to current brightness progress on primary
    tubes[_activeTube].ActiveCathode = tubes[_activeTube].PrimaryCathode;
    tubes[_activeTube].Brightness = _tubeProgress[_activeTube];

    // pick a tube that is not current tube and is not fully bright
    int8_t nextTube = -1;
    int tries = 0;
    do {
      nextTube = random(NUM_TUBES);
    } while((nextTube == _activeTube || _tubeProgress[nextTube] == PWM_MAX) && tries++ < 1000);

    // randomly pick a cathode for tube's type
    TubeType type = tubes[nextTube].Type;
    int nextCathodeIndex = random(TubeCathodeCount[type]);
    tubes[nextTube].ActiveCathode = TubeCathodes[type][nextCathodeIndex];
    tubes[nextTube].Brightness = PWM_MAX;

    if (tubes[nextTube].ActiveCathode == tubes[nextTube].PrimaryCathode) {
      if (_tubeProgress[nextTube] <= 0) {
        _tubeProgress[nextTube] = 1;
      }
      else { 
        _tubeProgress[nextTube] <<= 2;
      }

      if (_tubeProgress[nextTube] > PWM_MAX) {
        _tubeProgress[nextTube] = PWM_MAX;
      }

      bool isComplete = true;
      for (int i = 0; i < NUM_TUBES; i++) {
        isComplete &= _tubeProgress[i] == PWM_MAX;
      }

      if (isComplete) {
        setComplete();
      }
    }

    _activeTube = nextTube;
    _cathodeDelay = 30;
    result = {true, true};
  }

  return result;
}