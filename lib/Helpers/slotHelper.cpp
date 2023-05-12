#include <Arduino.h>

#include "slotHelper.h"

void SlotHelper::disableSlot(int tubeIndex) {
  _tubeSlotActive[tubeIndex] = false;
}

void SlotHelper::disableAllSlots() {
  for (int i = 0; i < NUM_TUBES; i++) {
    disableSlot(i);
  }
}

void SlotHelper::enableSlot(int tubeIndex) {
  _tubeSlotActive[tubeIndex] = true;
}

void SlotHelper::enableAllSlots(int tubeIndex) {
  for (int i = 0; i < NUM_TUBES; i++) {
    enableSlot(i);
  }
}

bool SlotHelper::isSlotEnabled(int tubeIndex) {
  return _tubeSlotActive[tubeIndex];
}

bool SlotHelper::handleTick(Tube tubes[NUM_TUBES]) {
  bool update = false;

  if (_slotDelay < 0) {
    for (int i = 0; i < NUM_TUBES; i++) {
      if (_tubeSlotActive[i]) {
        _cathodeIndex[i] = (_cathodeIndex[i] + 1) % TubeCathodeCount[tubes[i].Type];
        tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][_cathodeIndex[i]];
      }
      else if (tubes[i].ActiveCathode != tubes[i].PrimaryCathode) {
        tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
      }
    }

    _slotDelay = DefaultDelayMs;
    update = true;
  }

  return update;
}