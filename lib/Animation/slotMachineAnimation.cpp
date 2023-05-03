#include <Arduino.h>

#include "slotMachineAnimation.h"

void SlotMachineAnimation::initialize(Tube tubes[NUM_TUBES]) {
  Serial.println("SlotMachineAnimation::initialize");
  Animation::setDuration(14440);

  _cathodeDelay = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    //_cathodeIndex[i] = TubeCathodes[tubes[i].Type][random(TubeCathodeCount[tubes[i].Type])];
    _tubePhaseOffsetDeg[i] = (360 / NUM_TUBES) * i;
    _tubeSlotActive[i] = false;
  }

  _pwmDelay = 0;

  _brightnessMin = 80;
  _brightnessMax = PWM_MAX;
  _brightnessPhaseDeg = 0;
  _brightnessPeriodMs = random(500,2500);
  _brightnessPhaseStepMs = _brightnessPeriodMs / BrightnessPeriodSteps;

  _totalCyclesLeft = random(3, 5);
}

TickResult SlotMachineAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool brightnessUpdate = false;
  bool cathodeUpdate = false;
  _cathodeDelay--;
  _pwmDelay--;

  if (_pwmDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      int tubePhaseDeg = _brightnessPhaseDeg + _tubePhaseOffsetDeg[i];

      if (!_tubeSlotActive[i]) {
        // activate tubes at starting phase of left-most tube
        if (tubePhaseDeg == _tubePhaseOffsetDeg[NUM_TUBES-1] && _totalCyclesLeft > 0) {
          _tubeSlotActive[i] = true;
        }
        else {
          continue;
        }
      }
      
      if (_totalCyclesLeft < 0 && tubePhaseDeg == _tubePhaseOffsetDeg[NUM_TUBES-1]) {
        tubes[i].Brightness = 150; // TODO: need to do the configable max brightness
        _tubeSlotActive[i] = false;

        if (i == 0) {
          setDuration(-1);
        }

        continue;
      }

      float tubePhaseRad = tubePhaseDeg * M_PI / 180;
      tubes[i].Brightness = _brightnessMin + (_brightnessMax - _brightnessMin) * sin(tubePhaseRad);
    }

    _brightnessPhaseDeg = (_brightnessPhaseDeg + BrightnessPhaseStepDeg) % 360;
    _pwmDelay = _brightnessPhaseStepMs;
    brightnessUpdate = true;

    if (_brightnessPhaseDeg == 0) {
      _totalCyclesLeft--;
    }
  }

  if (_cathodeDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      if (!_tubeSlotActive[i]) {
        if (_totalCyclesLeft < 0) {
          tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
        }
        continue;
      }

      _cathodeIndex[i] = (_cathodeIndex[i] + 1) % TubeCathodeCount[tubes[i].Type];
      tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][_cathodeIndex[i]];
    }

    _cathodeDelay = DefaultCathodeDelay;
    cathodeUpdate = true;
  }

  return {cathodeUpdate, brightnessUpdate};
}
