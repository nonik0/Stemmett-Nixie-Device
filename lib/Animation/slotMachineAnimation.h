#include "animationBase.h"
#include "tubes.h"

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
  void initialize(Tube tubes[NUM_TUBES]);
  TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
