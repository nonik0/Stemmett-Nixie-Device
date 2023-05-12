#include <Arduino.h>

#include "cylonScanANimation.h"

void CylonScanAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("CylonScanAnimation::initialize");
  Animation::setDuration(10000);
  Animation::initialize(tubes, maxBrightness);
  for (int i = 0; i < NUM_TUBES; i++) {
    //_tubeProgress[i] = 0;
    //tubes[i].Brightness = 0;
  }
}

TickResult CylonScanAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  TickResult result = {false, false};
  return result;
}