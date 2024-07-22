#include "animationBase.h"
#include "tubes.h"

class RandomScanAnimation : public Animation {
 private:
  int8_t _activeTube;
  int32_t _cathodeDelay;
  int _tubeProgress[NUM_TUBES];

 public:
  void initialize(Tube tubes[NUM_TUBES] , int maxBrightness, float speedFactor) override;
  TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
