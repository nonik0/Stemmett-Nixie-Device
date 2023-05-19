#include <Arduino.h>

#include "pulsarAnimation.h"

void PulsarAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("PulsarAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness);

  _ejectionDelay = 1000;
  _ejectionDelayRange = 1000;
  for (int i = 0; i < NumEjections; i++) {
    ejection[i].isActive = false;
  }

  _pulsarIndex = 1;

  _slotHelper.resetDisabledSlotsToDefault(false);
  _slotHelper.enableSlot(_pulsarIndex, 4);
}


TickResult PulsarAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

    // pulsar movement
  // _movementDelay--;
  // if (_movementDelay < 0) {
    
  // }

  // spawn new ejection when timer up
  _ejectionDelay--;
  if (_ejectionDelay < 0) {
    int inactiveEjectionIndex = -1;
    for (int i = 0; i < NumEjections; i++) {
      if (!ejection[i].isActive) {
        inactiveEjectionIndex = i;
        break;
      }
    }

    // spawn new only if not at max active count
    if (inactiveEjectionIndex >= 0) {
      ejection[inactiveEjectionIndex].isActive = true;
      ejection[inactiveEjectionIndex].delay = 0;
      ejection[inactiveEjectionIndex].index = _pulsarIndex;
      ejection[inactiveEjectionIndex].brightness = random(_maxBrightness / 2, _maxBrightness);
      ejection[inactiveEjectionIndex].speed = random(30, 150);
      ejection[inactiveEjectionIndex].direction = random(2) == 0 ? Left : Right;
      ejection[inactiveEjectionIndex].distanceLeft = random(3,NUM_TUBES); // 1 + random(NUM_TUBES);
      ejection[inactiveEjectionIndex].slotActive = random(2) == 0;
      // todo: fade
    }

    _ejectionDelay = _ejectionDelayRange; //random(0,_ejectionFrequencyRange); // TODO: random or periodic?
  }

  bool cathodeUpdate = false;
  bool brightnessUpdate = false;

  for (int i = 0; i < NumEjections; i++) {
    if (ejection[i].isActive) {
      ejection[i].delay--;

      if (ejection[i].delay < 0) {
        // old tube location updates (skip first location at pulsar)
        if (ejection[i].index != _pulsarIndex) {
          if (ejection[i].slotActive) {
            _slotHelper.disableSlot(ejection[i].index);
          }
          tubes[ejection[i].index].Brightness = 0; // TODO: combine with other ejections
          brightnessUpdate = true;
        }

        // update ejection location
        ejection[i].delay = ejection[i].speed;
        ejection[i].index += ejection[i].direction;
        ejection[i].distanceLeft--;
        
        // several checks to make sure ejection is still active, then update new tube location
        if (ejection[i].index >= 0 && ejection[i].index <= NUM_TUBES - 1 && ejection[i].distanceLeft >= 0) {
          // new tube positions updates
          if (ejection[i].slotActive) {
            _slotHelper.enableSlot(ejection[i].index, 50); // TODO: varied cycle time?
          }
          else {
            uint8_t newCathode = TubeCathodes[tubes[i].Type][random(TubeCathodeCount[tubes[i].Type])]; // TODO: helper function?
            tubes[ejection[i].index].ActiveCathode = newCathode;
            cathodeUpdate = true;
          }

          tubes[ejection[i].index].Brightness = ejection[i].brightness; // TODO: combine with other ejections
          brightnessUpdate = true;
        }
        else {
          ejection[i].isActive = false;
        }
      }
    }
  }

  cathodeUpdate |= _slotHelper.handleTick(tubes);
  brightnessUpdate |= _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}