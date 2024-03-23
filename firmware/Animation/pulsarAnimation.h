#include <Arduino.h>

#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "Helpers/slotHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  int delay;
  int index;
  int brightness;
  int speed; // delayReset
  Direction direction;
  int distance;
  bool slotActive;
} Ejection;

class PulsarAnimation : public Animation {
  private:
    const int StartOffset = 4;
    const int EndOffset = 2;
    bool _finalEjection;
    int _pulsarIndex;
    int _pulsarMovementDelay;
    int _pulsarSpeed;
    Direction _pulsarDirection;
    int _pulsarSlotDelay[NUM_TUBES];
    static const int NumEjections = 7;
    Ejection ejection[NumEjections];
    int _ejectionDelay;
    int _ejectionDelayRange;
    
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;

    void handlePulsarMovement();
    void handleEjectionSpawning();
    void handleEjectionMovement();
    bool isVisible(int index);
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
