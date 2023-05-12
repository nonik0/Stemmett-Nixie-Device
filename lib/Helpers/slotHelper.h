#pragma once

#include "tubes.h"

class SlotHelper {
  private:
    const int DefaultDelayMs = 45;
    // todo: combine into struct
    bool _tubeSlotActive[NUM_TUBES];
    uint8_t _cathodeIndex[NUM_TUBES];
    int _slotDelay;
  public:
    //SlotHelper();
    void disableSlot(int tubeIndex);
    void disableAllSlots();
    void enableSlot(int tubeIndex);
    void enableAllSlots(int tubeIndex);
    bool isSlotEnabled(int tubeIndex);
    bool handleTick(Tube tubes[NUM_TUBES]);
};
