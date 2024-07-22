#include <Arduino.h>

#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "Helpers/slotHelper.h"
#include "tubes.h"

class CylonScanAnimation : public Animation {
  private:
    int _activePhaseDuration;
    bool _isActivePhase;
    int _eyeFadeDuration;
    int _eyeShiftDelay;
    int _eyeIndex;
    int _slotDelay;
    Direction _eyeDirection;
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;

    int _eyeDelay;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
