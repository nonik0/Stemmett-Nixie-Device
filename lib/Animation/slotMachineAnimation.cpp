#include <Arduino.h>

#include "slotMachineAnimation.h"

void SlotMachineAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("SlotMachineAnimation::initialize");
  Animation::initialize(tubes, maxBrightness);
  Animation::setDuration(14440);

  _cathodeDelay = 0;
  _brightnessDelay = 0;

  for (int i = 0; i < NUM_TUBES; i++) {
    _tubePhaseOffsetDeg[i] = (360 / NUM_TUBES) * i;
    _tubeSlotActive[i] = false;
  }

  // random phase direction, length and number of periods
  int brightnessPeriodMs = random(500,2500);
  _brightnessPhaseStepMs = brightnessPeriodMs / BrightnessPeriodSteps;
  _totalCyclesLeft = random(3, 5);
  _direction = (random(2) == 0) ? -1 : 1; // direction < 0 => rightmost tube is "trigger" tube

  // want initial phase to be where the "trigger" tube is at max brightness (i.e. 0 deg phase with cos)
  int triggerTubePhaseOffset = (_direction < 0) ? _tubePhaseOffsetDeg[0] : _tubePhaseOffsetDeg[NUM_TUBES - 1];
  _brightnessInitialPhaseDeg = (360 - triggerTubePhaseOffset) % 360;
  _brightnessPhaseDeg = _brightnessInitialPhaseDeg;
  _tubeTriggerPhase = 0; // cos max phase

  // have brightness go negative to increase depth of animeation
  _brightnessMin = -25;
  _brightnessMax = _maxBrightness;

  // original values:
  //_brightnessMin = -95;
  //_brightnessMax = 255; // TODO

  // +: 255/-95
  // -: 152/-50
  // +: 152/-38
}

TickResult SlotMachineAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool brightnessUpdate = false;
  bool cathodeUpdate = false;
  _cathodeDelay--;
  _brightnessDelay--;

  if (_brightnessDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      int tubePhaseDeg = (_brightnessPhaseDeg + _tubePhaseOffsetDeg[i]) % 360;

      if (!_tubeSlotActive[i]) {
        // activate tubes at starting phase of left-most tube
        if (tubePhaseDeg == _tubeTriggerPhase && _totalCyclesLeft > 0) {
          _tubeSlotActive[i] = true;
        }
        else {
          continue;
        }
      }
      
      if (_totalCyclesLeft < 0 && tubePhaseDeg == _tubeTriggerPhase) {
        tubes[i].Brightness = _maxBrightness;
        _tubeSlotActive[i] = false;

        if ((_direction < 0 && i == (NUM_TUBES - 1)) || (_direction > 0 && i == 0)) {
          setDuration(-1);
        }

        continue;
      }

      float tubePhaseRad = tubePhaseDeg * M_PI / 180;

      //tubes[i].Brightness = _brightnessMin + (_brightnessMax - _brightnessMin) * cos(tubePhaseRad);
      tubes[i].Brightness = _brightnessMin + 0.5 * (_brightnessMax - _brightnessMin) * (1 + cos(tubePhaseRad));
    }

    _brightnessPhaseDeg = (360 + _brightnessPhaseDeg + _direction * BrightnessPhaseStepDeg) % 360;
    _brightnessDelay = _brightnessPhaseStepMs;
    brightnessUpdate = true;

    if (_brightnessPhaseDeg == _brightnessInitialPhaseDeg) {
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
