#include <Arduino.h>

#include "animationBase.h"
#include "fadeHelper.h"
#include "tubes.h"

typedef enum { Left, Right} Direction;

class CylonScanAnimation : public Animation {
  private:
    int _eyeIndex;
    Direction _eyeDirection; // true = right, false = left
    FadeHelper _fadeHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
