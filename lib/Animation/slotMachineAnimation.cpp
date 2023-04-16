#include <Arduino.h>

#include "slotMachineAnimation.h"

void SlotMachineAnimation::initialize(Tube tubes[NUM_TUBES]) {
  Animation::setDuration(7200);

  _cathodeDelay = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    _cathodeIndex[i] = TubeCathodes[tubes[i].Type][random(TubeCathodeCount[tubes[i].Type])];
  }

  _pwmDelay = 0;
  _brightnessMin = 80;
  _brightnessMax = PWM_MAX;
  _brightnessPhaseDeg = 0;
  _brightnessPeriodMs = random(500,1500);
  _brightnessPhaseStepMs = _brightnessPeriodMs / BrightnessPeriodSteps;
}

TickResult SlotMachineAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  TickResult result = {false, false};
  _cathodeDelay--;
  _pwmDelay--;

  if (_cathodeDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      _cathodeIndex[i] = (_cathodeIndex[i] + 1) % TubeCathodeCount[tubes[i].Type];
      tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][_cathodeIndex[i]];
    }

    _cathodeDelay = 45;
    result.CathodeUpdate = true;
  }

  if (_pwmDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      int tubePhaseOffsetDeg = (360 / NUM_TUBES) * i;
      float tubePhaseRad =
          (_brightnessPhaseDeg + tubePhaseOffsetDeg) * M_PI / 180;
      tubes[i].Brightness = _brightnessMin + (_brightnessMax - _brightnessMin) * sin(tubePhaseRad);
    }

    _brightnessPhaseDeg = (_brightnessPhaseDeg + BrightnessPhaseStepDeg) % 360;
    _pwmDelay = _brightnessPhaseStepMs;
    result.BrightnessUpdate = true;
  }

  return result;
}
