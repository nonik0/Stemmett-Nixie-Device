#include <Arduino.h>

#include "pulsarAnimation.h"

bool PulsarAnimation::isVisible(int index) {
  return index >= 0 && index <= NUM_TUBES - 1;
}

int PulsarAnimation::getSlotDelay(TubeType tubeType) {
  int numCathodes = TubeCathodeCount[tubeType];
  return 14 - numCathodes;
}

void PulsarAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("PulsarAnimation::initialize");
  Animation::setDuration(25000);
  Animation::initialize(tubes, maxBrightness);

  _pulsarIndex = 0;//(random(2) == 0) ? -2 : NUM_TUBES + 1;
  _speed = random(1000,4000);
  _movementDelay = _speed;
  _direction = _pulsarIndex <= 0 ? Left : Right;

  _ejectionDelay = 0;
  _ejectionDelayRange = 100;
  for (int i = 0; i < NumEjections; i++) {
    ejection[i].isActive = false;
  }
}

void PulsarAnimation::handlePulsarMovement() {
  _movementDelay--;
  if (_movementDelay < 0) {
    if (isVisible(_pulsarIndex)) {
      _slotHelper.disableCycling(_pulsarIndex);
      _fadeHelper.setTubeBrightness(_pulsarIndex, 0);
    }

    _pulsarIndex += _direction;
    _movementDelay = _speed;

    if (isVisible(_pulsarIndex)) {
      _slotHelper.enableCycling(_pulsarIndex, 6); // TODO getSlotDelay(tubes[_pulsarIndex].Type);
      _fadeHelper.setTubeBrightness(_pulsarIndex, _maxBrightness);
    }
  }

  // TODO
  if (_pulsarIndex < -1 || _pulsarIndex > NUM_TUBES) {
    Animation::setDuration(-1);
  }
}

void PulsarAnimation::handleEjectionSpawning() {
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
      // if pulsar is not visible then ejections travels shorter distance
      int ejectionDistance = isVisible(_pulsarIndex) ? random(2, NUM_TUBES-1) : random(1,3);

      ejection[inactiveEjectionIndex].isActive = true;
      ejection[inactiveEjectionIndex].delay = 0;
      ejection[inactiveEjectionIndex].index = _pulsarIndex;
      ejection[inactiveEjectionIndex].brightness = random(_maxBrightness / 2, _maxBrightness);
      ejection[inactiveEjectionIndex].speed = random(30, 300);
      ejection[inactiveEjectionIndex].direction = random(2) == 0 ? Left : Right;
      ejection[inactiveEjectionIndex].distance = ejectionDistance;
      ejection[inactiveEjectionIndex].slotActive = random(2) == 0;
      // TODO: fade?
    }

    _ejectionDelay = _ejectionDelayRange; //random(0,_ejectionFrequencyRange); // TODO: random or periodic?
  }
}

void PulsarAnimation::handleEjectionMovement() {
  for (int i = 0; i < NumEjections; i++) {
    if (ejection[i].isActive) {
      ejection[i].delay--;

      if (ejection[i].delay < 0) {
        // old tube location updates (skip first location at pulsar)
        if (ejection[i].index != _pulsarIndex) {
          if (ejection[i].slotActive) {
            _slotHelper.disableCycling(ejection[i].index);
          }
          _fadeHelper.setTubeBrightness(ejection[i].index, 0);
        }

        // update ejection location
        ejection[i].delay = ejection[i].speed;
        ejection[i].index += ejection[i].direction;
        ejection[i].distance--;
        
        // several checks to make sure ejection is still active, then update new tube location
        if (ejection[i].index >= 0 && ejection[i].index <= NUM_TUBES - 1 && ejection[i].distance >= 0) {
          // new tube positions updates
          if (ejection[i].slotActive) {
            _slotHelper.enableCycling(ejection[i].index, 50); // TODO: varied cycle time?
          }
          else {
            _slotHelper.setRandomCathode(ejection[i].index);
          }

          _fadeHelper.setTubeBrightness(ejection[i].index, ejection[i].brightness);
        }
        else {
          ejection[i].isActive = false;
        }
      }
    }
  }
}

TickResult PulsarAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  handlePulsarMovement();
  handleEjectionSpawning();
  handleEjectionMovement();

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}