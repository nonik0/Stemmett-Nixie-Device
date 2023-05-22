#include <Arduino.h>

#include "slotHelper.h"

void SlotHelper::disableCycling(int tubeIndex, bool resetToDefault) {
  _slotState[tubeIndex].action = resetToDefault ? PrimaryCathode : RandomCathode;
}

void SlotHelper::enableCycling(int tubeIndex, int cycleDelay) {
  _slotState[tubeIndex].action = SeqCycle;
  _slotState[tubeIndex].cathodeIndex = 0;
  _slotState[tubeIndex].cycleDelay = 0;
  _slotState[tubeIndex].cycleDelayReset = cycleDelay;
}

bool SlotHelper::isSlotEnabled(int tubeIndex) {
  return _slotState[tubeIndex].action != Inactive;
}

void SlotHelper::setPrimaryCathode(int tubeIndex) {
  _slotState[tubeIndex].action == PrimaryCathode;
}

void SlotHelper::setRandomCathode(int tubeIndex) {
  _slotState[tubeIndex].action == RandomCathode;
}

bool SlotHelper::handleTick(Tube tubes[NUM_TUBES]) {
  bool update = false;

  for (int i = 0; i < NUM_TUBES; i++) {
    if (_slotState[i].action == SeqCycle) {
      _slotState[i].cycleDelay--;

      if (_slotState[i].cycleDelay < 0) {
        _slotState[i].cathodeIndex = (_slotState[i].cathodeIndex + 1) % TubeCathodeCount[tubes[i].Type];
        _slotState[i].cycleDelay = _slotState[i].cycleDelayReset;
        tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][_slotState[i].cathodeIndex];
        update = true;
      }
    }
    // else if (_slotState[i].action == RandomCycle) {
    // }
    else if (_slotState[i].action == RandomCathode) {
      //tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][_slotState[i].cathodeIndex];
      tubes[i].ActiveCathode = TubeCathodes[tubes[i].Type][random(TubeCathodeCount[tubes[i].Type])];
      _slotState[i].action = Inactive;
      update = true;
    }
    else if (_slotState[i].action == PrimaryCathode) {
      tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
      _slotState[i].action = Inactive;
      update = true;
    }
  }

  return update;
}