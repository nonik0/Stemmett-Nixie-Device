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
  int distance;
  bool slotActive;
} Ejection;

class PulsarAnimation : public Animation {
  private:
    int _pulsarIndex;
    int _movementDelay;
    int _speed;   
    Direction _direction;
    static const int NumEjections = 3;
    Ejection ejection[NumEjections];
    int _ejectionDelay;
    int _ejectionDelayRange;
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;

    void handlePulsarMovement(Tube tubes[NUM_TUBES]);
    void handleEjectionSpawning();
    void handleEjectionMovement();
    bool isVisible(int index); 
    int getSlotDelay(TubeType tubeType);
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
