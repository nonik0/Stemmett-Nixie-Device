#pragma once

#include "tubes.h"

// animation ideas:
// slot machine:
// - one cathode active each tube
// - one cathode active all tubes
// - stop sequence:
//   - end left to right each primary cathode
//   - end random tube each priamry cathode

typedef struct {
  bool CathodeUpdate;
  bool BrightnessUpdate;
} TickResult;

class Animation {
 private:
  int _duration = 3600;

 public:
  virtual ~Animation() = default;
  virtual void initialize() = 0;
  virtual TickResult handleTick() = 0;
  bool isComplete();
};

void Animation::initialize() { _duration = 3600; }

TickResult Animation::handleTick() {
  _duration--;
  return {false, false};
}
bool Animation::isComplete() {
  return _duration < 0;
}

class SlotMachineAnimation : public Animation {
 private:
  const int BrightnessPeriodSteps = 90;
  const int BrightnessPhaseStepDeg = 360 / BrightnessPeriodSteps;

  int _cathodeDelay;
  int _pwmDelay;
  int _brightnessMin;
  int _brightnessMax;
  int _brightnessPhaseDeg;
  int _brightnessPeriodMs;
  int _brightnessPhaseStepMs;

 public:
  SlotMachineAnimation();
  void initialize();
  TickResult handleTick() override;
};

SlotMachineAnimation::SlotMachineAnimation() { initialize(); }

void SlotMachineAnimation::initialize() {
  Animation::initialize();

  _cathodeDelay = 0;
  _pwmDelay = 0;
  _brightnessMin = 80;
  _brightnessMax = PWM_MAX;
  _brightnessPhaseDeg = 0;
  _brightnessPeriodMs = 900;
  _brightnessPhaseStepMs = _brightnessPeriodMs / BrightnessPeriodSteps;
}

TickResult SlotMachineAnimation::handleTick() {
  Animation::handleTick();

  TickResult result = {false, false};
  _cathodeDelay--;
  _pwmDelay--;

  // Serial.printf("_cathodeDelay:%d, _pwmDelay:%d\n", _cathodeDelay,
  // _pwmDelay);

  if (_cathodeDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      Tubes[i].ActiveCathode =
          (Tubes[i].ActiveCathode + 1) % TubeCathodeCount[Tubes[i].Type];
    }

    // Serial.printf("K:%d|%d|%d|%d|%d|%d\n", Tubes[5].ActiveCathode,
    // Tubes[4].ActiveCathode, Tubes[3].ActiveCathode, Tubes[2].ActiveCathode,
    // Tubes[1].ActiveCathode, Tubes[0].ActiveCathode);
    _cathodeDelay = 45;
    result.CathodeUpdate = true;
  }

  if (_pwmDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      int tubePhaseOffsetDeg = (360 / NUM_TUBES) * i;
      float tubePhaseRad =
          (_brightnessPhaseDeg + tubePhaseOffsetDeg) * M_PI / 180;
      Tubes[i].Brightness = _brightnessMin + (_brightnessMax - _brightnessMin) * sin(tubePhaseRad);
    }

    _brightnessPhaseDeg = (_brightnessPhaseDeg + BrightnessPhaseStepDeg) % 360;
    _pwmDelay = _brightnessPhaseStepMs;
    result.BrightnessUpdate = true;
  }

  return result;
}