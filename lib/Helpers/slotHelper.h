#pragma once

#include "tubes.h"

typedef struct {
  bool isActive;
  uint8_t cathodeIndex;
  int cycleDelay;
  int cycleDelayReset;
} SlotState;

class SlotHelper {
  private:
    static const int DefaultCycleDelayMs = 45;
    bool _resetToDefault = true;
    SlotState _slotState[NUM_TUBES];
  public:
    void disableSlot(int tubeIndex);
    void disableAllSlots();
    void enableSlot(int tubeIndex, int cycleDelay = DefaultCycleDelayMs);
    void enableAllSlots(int cycleDelay = DefaultCycleDelayMs);
    bool isSlotEnabled(int tubeIndex);
    bool handleTick(Tube tubes[NUM_TUBES]);
    void resetDisabledSlotsToDefault(bool isEnabled);
};
