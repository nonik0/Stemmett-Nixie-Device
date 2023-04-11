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
  int _durationMs = 3600;

 public:
  virtual ~Animation() = default;
  virtual void initialize() = 0;
  virtual TickResult handleTick() = 0;
  bool isComplete();
  void setDuration(int durationMs);
};

void Animation::setDuration(int durationMs) { _durationMs = durationMs; }

TickResult Animation::handleTick() {
  _durationMs--;
  return {false, false};

}
bool Animation::isComplete() {
  return _durationMs < 0;
}


class PrimaryCathodeAnimation : public Animation {
  private:
    bool _firstTick;
  public:
    PrimaryCathodeAnimation();
    void initialize() override;
    TickResult handleTick() override;
};

PrimaryCathodeAnimation::PrimaryCathodeAnimation() {
  initialize();
}

void PrimaryCathodeAnimation::initialize() {
  Animation::setDuration(2000);
  _firstTick = true;
}

TickResult PrimaryCathodeAnimation::handleTick() {
  Animation::handleTick();

  if (_firstTick) {
    for (int i = 0; i < NUM_TUBES; i++) {
      Tubes[i].ActiveCathode = Tubes[i].PrimaryCathode;
      Tubes[i].Brightness = 150;
    }
    return {true,true};
  }

  return {false,false};
}


class SlotMachineAnimation : public Animation {
 private:
  const int BrightnessPeriodSteps = 90;
  const int BrightnessPhaseStepDeg = 360 / BrightnessPeriodSteps;

  int _cathodeDelay;
  uint8_t _cathodeIndex[NUM_TUBES];
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

SlotMachineAnimation::SlotMachineAnimation() {
  initialize();
}

void SlotMachineAnimation::initialize() {
  Animation::setDuration(3600);

  _cathodeDelay = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    _cathodeIndex[i] = TubeCathodes[Tubes[i].Type][random(TubeCathodeCount[Tubes[i].Type])];
  }

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

  if (_cathodeDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      _cathodeIndex[i] = (_cathodeIndex[i] + 1) % TubeCathodeCount[Tubes[i].Type];
      Tubes[i].ActiveCathode = TubeCathodes[Tubes[i].Type][_cathodeIndex[i]];
    }

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