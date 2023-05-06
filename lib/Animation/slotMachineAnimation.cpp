#include <Arduino.h>

#include "slotMachineAnimation.h"

void SlotMachineAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("SlotMachineAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(14440);

  _cathodeDelay = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    //_cathodeIndex[i] = TubeCathodes[tubes[i].Type][random(TubeCathodeCount[tubes[i].Type])];
    _tubePhaseOffsetDeg[i] = (360 / NUM_TUBES) * i;
    _tubeSlotActive[i] = false;
  }

  if (_direction) {
    _triggerPhase = _tubePhaseOffsetDeg[NUM_TUBES-1];
  }
  else {
    _triggerPhase = _tubePhaseOffsetDeg[0];
  }

  _brightnessDelay = 0;

  _brightnessMin = 80;
  _brightnessMax = PWM_MAX; // TODO
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
  _brightnessDelay--;

  if (_brightnessDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      int tubePhaseDeg = _brightnessPhaseDeg + _tubePhaseOffsetDeg[i];

      if (!_tubeSlotActive[i]) {
        // activate tubes at starting phase of left-most tube
        if (tubePhaseDeg == _triggerPhase && _totalCyclesLeft > 0) {
          _tubeSlotActive[i] = true;
        }
        else {
          continue;
        }
      }
      
      if (_totalCyclesLeft < 0 && tubePhaseDeg == _triggerPhase) {
        tubes[i].Brightness = _maxBrightness;
        _tubeSlotActive[i] = false;

        if (i == 0) {
          setDuration(-1);
        }

        continue;
      }

      float tubePhaseRad = tubePhaseDeg * M_PI / 180;
      // TODO: fix
      tubes[i].Brightness = _brightnessMin + (_brightnessMax - _brightnessMin) * sin(tubePhaseRad);
      //tubes[i].Brightness = _brightnessMin + 0.5 * (_brightnessMax - _brightnessMin) * (1 + sin(tubePhaseRad));
    }

    _brightnessPhaseDeg = _direction
      ? (_brightnessPhaseDeg + BrightnessPhaseStepDeg) % 360
      : (_brightnessPhaseDeg - BrightnessPhaseStepDeg) % 360;
    _brightnessDelay = _brightnessPhaseStepMs;
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

    _cathodeDelay = DefaultCathodeDelayMs;
    cathodeUpdate = true;
  }

  return {cathodeUpdate, brightnessUpdate};
}
