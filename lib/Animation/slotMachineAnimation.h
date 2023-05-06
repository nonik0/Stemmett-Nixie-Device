#include "animationBase.h"
#include "tubes.h"

class SlotMachineAnimation : public Animation {
 private:
  const int BrightnessPeriodSteps = 90;
  const int BrightnessPhaseStepDeg = 360 / BrightnessPeriodSteps;
  const int DefaultCathodeDelayMs = 45;

  int _cathodeDelay;
  uint8_t _cathodeIndex[NUM_TUBES];
  int _tubePhaseOffsetDeg[NUM_TUBES];
  int _brightnessDelay;
  int _brightnessMin;
  int _brightnessMax;
  int _brightnessPhaseDeg;
  int _brightnessPeriodMs;
  int _brightnessPhaseStepMs;
  
  bool _direction;
  // used for fading in and out
  bool _tubeSlotActive[NUM_TUBES];
  int _totalCyclesLeft;
  int _triggerPhase;
 public:
  void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
  TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
