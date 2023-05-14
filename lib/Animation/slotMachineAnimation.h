#include "animationBase.h"
#include "slotHelper.h"
#include "tubes.h"

class SlotMachineAnimation : public Animation {
 private:
  const int BrightnessPeriodSteps = 60;
  const int BrightnessPhaseStepDeg = 360 / BrightnessPeriodSteps;
  const int DefaultCathodeDelayMs = 45;

  int _tubePhaseOffsetDeg[NUM_TUBES];
  int _brightnessDelay;
  int _brightnessMin;
  int _brightnessMax;
  int _brightnessInitialPhaseDeg;
  int _brightnessPhaseDeg;
  int _brightnessPhaseStepMs;
  Direction _direction;
  
  // used for fading in and out
  int _totalCyclesLeft;
  int _tubeTriggerPhase;

  SlotHelper _slotHelper;
 public:
  void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
  TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
