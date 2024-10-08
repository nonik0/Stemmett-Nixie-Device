#include <Arduino.h>

#include "slotMachineAnimation.h"

void SlotMachineAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("SlotMachineAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(26000);

  _brightnessDelay = 0;

  for (int i = 0; i < NUM_TUBES; i++) {
    _tubePhaseOffsetDeg[i] = (360 / NUM_TUBES) * i;
  }

  // random phase direction, length and number of periods
  int minBrightnessPeriodMs = 500 + 500 * (1 - speedFactor); // slowest: 1000, fastest: 500
  int maxBrightnessPeriodMs = 2000 + 1000 * (1 - speedFactor); // slowest: 3000, fastest: 2000
  int brightnessPeriodMs = random(minBrightnessPeriodMs, maxBrightnessPeriodMs);
  _brightnessPhaseStepMs = brightnessPeriodMs / BrightnessPeriodSteps;
  _totalCyclesLeft = random(6, 10);
  _direction = random(2) == 0 ? Left : Right;

  // want initial phase to be where the "trigger" tube is at max brightness (i.e. 0 deg phase with cos)
  int triggerTubePhaseOffset = (_direction == Left) ? _tubePhaseOffsetDeg[0] : _tubePhaseOffsetDeg[NUM_TUBES - 1];
  _brightnessInitialPhaseDeg = (360 - triggerTubePhaseOffset) % 360;
  _brightnessPhaseDeg = _brightnessInitialPhaseDeg;
  _tubeTriggerPhase = 0; // cos max phase

  // have brightness go negative to prolong effect of fade
  _brightnessMin = random(-50, -10);
  _brightnessMax = _maxBrightness;

  //log_i("_brightnessMin: %d", _brightnessMin);
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
  _brightnessDelay--;

  if (_brightnessDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      int tubePhaseDeg = (_brightnessPhaseDeg + _tubePhaseOffsetDeg[i]) % 360;

      if (!_slotHelper.isSlotCycling(i)) {
        // activate tubes at starting phase of left-most tube
        if (tubePhaseDeg == _tubeTriggerPhase && _totalCyclesLeft > 0) {
          int cycleDelay = 45 + 45 * (1 - _speedFactor); // slowest: 90, fastest: 45
          _slotHelper.enableCycling(i, cycleDelay);
        }
        else {
          continue;
        }
      }
      
      if (_totalCyclesLeft < 0 && tubePhaseDeg == _tubeTriggerPhase) {
        tubes[i].Brightness = _maxBrightness;
        _slotHelper.setPrimaryCathode(i);

        if ((_direction == Left && i == (NUM_TUBES - 1)) || (_direction == Right && i == 0)) {
          setComplete();
        }

        continue;
      }

      float tubePhaseRad = tubePhaseDeg * M_PI / 180;

      //tubes[i].Brightness = _brightnessMin + (_brightnessMax - _brightnessMin) * cos(tubePhaseRad);
      tubes[i].Brightness = _brightnessMin + 0.5 * (_brightnessMax - _brightnessMin) * (1 + cos(tubePhaseRad));
    }

    _brightnessPhaseDeg = (360 + _brightnessPhaseDeg + -_direction * BrightnessPhaseStepDeg) % 360;
    _brightnessDelay = _brightnessPhaseStepMs;
    brightnessUpdate = true;

    if (_brightnessPhaseDeg == _brightnessInitialPhaseDeg) {
      _totalCyclesLeft--;
    }
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}
