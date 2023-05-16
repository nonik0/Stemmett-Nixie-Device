#include <Arduino.h>

#include "animationBase.h"
#include "fadeHelper.h"
#include "slotHelper.h"
#include "tubes.h"

class PulsarAnimation : public Animation {
  private:
    int _activePhaseDuration;
    bool _isActivePhase;

    int _pulsarEjectionDelay;
    int _pulsarEjectionFrequency;
    int _pulsarEjectionLength;
    int _pulsearEjectionSpeed;
    bool _pulsarEjectionActive; // TODO: multiple ejections?
    bool _pulsarEjectionSlotActive;

    int _pulsarMovementDelay;
    int _pulsarIndex;
    int _pulsarDelay;
    //Direction _pulsarDirection;
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;

    int _eyeDelay;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
