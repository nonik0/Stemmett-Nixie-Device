#include <Arduino.h>

#include "animationBase.h"
#include "fadeHelper.h"
#include "slotHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  int delay;
  int index;
  int brightness;
  int speed; // delayReset
  Direction direction;
  int distanceLeft;
  bool slotActive;
} Ejection;

class PulsarAnimation : public Animation {
  private:
    static const int NumEjections = 1;
    int _activePhaseDuration;
    bool _isActivePhase;

    Ejection ejection[NumEjections];
    int _ejectionDelay;
    int _ejectionDelayRange;

    int _pulsarIndex;
    int _movementDelay;
    Direction _direction;
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;

    int _eyeDelay;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
