#include <Arduino.h>

#include "slotHelper.h"

void SlotHelper::disableSlot(int tubeIndex) {
  Serial.printf("Disabling slot %d\n", tubeIndex);
  _slotState[tubeIndex].isActive = false;
}

void SlotHelper::disableAllSlots() {
  for (int i = 0; i < NUM_TUBES; i++) {
    disableSlot(i);
  }
}

void SlotHelper::enableSlot(int tubeIndex, int cycleDelay) {
  Serial.printf("Enabling slot %d\n", tubeIndex);
  _slotState[tubeIndex].isActive = true;
  _slotState[tubeIndex].cathodeIndex = 0;
  _slotState[tubeIndex].cycleDelay = 0;
  _slotState[tubeIndex].cycleDelayReset = cycleDelay;
}

void SlotHelper::enableAllSlots(int cycleDelay) {
  for (int i = 0; i < NUM_TUBES; i++) {
    enableSlot(i, cycleDelay);
  }
}

bool SlotHelper::isSlotEnabled(int tubeIndex) {
  return _slotState[tubeIndex].isActive;
}

bool SlotHelper::handleTick(Tube tubes[NUM_TUBES]) {
  bool update = false;

  for (int i = 0; i < NUM_TUBES; i++) {
    if (_slotState[i].isActive) {
      _slotState[i].cycleDelay--;

      if (_slotState[i].cycleDelay < 0) {
        _slotState[i].cathodeIndex = (_slotState[i].cathodeIndex + 1) % TubeCathodeCount[tubes[i].Type];
        _slotState[i].cycleDelay = _slotState[i].cycleDelayReset;
        tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][_slotState[i].cathodeIndex];
        update = true;
      }
    }
    else if (tubes[i].ActiveCathode != tubes[i].PrimaryCathode) {
      Serial.printf("Setting slot to default: %d\n", i);
      tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
      update = true;
    }
  }

  return update;
}