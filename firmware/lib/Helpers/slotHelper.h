#pragma once

#include "tubes.h"

typedef enum {
  Inactive,
  SeqCycle,
  //RandomCycle, // TODO: implement
  RandomCathode,
  PrimaryCathode
} SlotAction;

typedef struct {
  SlotAction action;
  uint8_t cathodeIndex;
  int cycleDelay;
  int cycleDelayReset;
} SlotState;

class SlotHelper {
  private:
    static const int DefaultCycleDelayMs = 45;
    SlotState _slotState[NUM_TUBES];
  public:
    void disableCycling(int tubeIndex, bool resetToDefault = true);
    void enableCycling(int tubeIndex, int cycleDelay = DefaultCycleDelayMs);
    bool isSlotCycling(int tubeIndex);
    void setPrimaryCathode(int tubeIndex);
    void setRandomCathode(int tubeIndex);
    bool handleTick(Tube tubes[NUM_TUBES]);
};
