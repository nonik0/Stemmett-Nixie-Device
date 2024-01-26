#pragma once

#include "tubes.h"

typedef struct {
  uint8_t targetBrightness;
  int stepDelay;
  uint8_t stepsLeft;
} TubeFadeState;

class FadeHelper {
  private:
    const int FadeStepDelay = 32; // TODO: needs to be different for different fade durations
    TubeFadeState _tubeFadeState[NUM_TUBES];
  public:
    FadeHelper();
    bool isComplete(int tubeIndex = -1);
    void setTubeBrightness(int tubeIndex, int targetBrightness);
    void setTubeFade(int tubeIndex, int targetBrightness, int durationMs, int initialDelay = 0);
    bool handleTick(Tube tubes[NUM_TUBES]);
};
