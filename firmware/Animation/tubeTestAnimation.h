#pragma once

#include <Arduino.h>

#include "animationBase.h"
#include "tubes.h"

class TubeTestAnimation : public Animation {
  private:
    static const int TEST_DURATION_MS = 12000;

    int _tickCount;
    int _currentCathodeIndex[NUM_TUBES];
    int _cathodeDisplayTicks[NUM_TUBES];  // ticks per cathode for each tube

  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
