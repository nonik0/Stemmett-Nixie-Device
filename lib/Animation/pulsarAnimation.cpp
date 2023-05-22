#include <Arduino.h>

#include "pulsarAnimation.h"

bool PulsarAnimation::isVisible(int index) {
  return index >= 0 && index <= NUM_TUBES - 1;
}

void PulsarAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
  Serial.println("PulsarAnimation::initialize");
  Animation::setDuration(30000);
  Animation::initialize(tubes, maxBrightness);

  _finalEjection = false;

  _pulsarIndex = (random(2) == 0) ? -StartOffset : NUM_TUBES - 1 + StartOffset;
  _pulsarSpeed = random(1000,3000);
  _pulsarMovementDelay = _pulsarSpeed;
  _pulsarDirection = _pulsarIndex <= 0 ? Left : Right;

  // pulsar cycling slot delay varies based on tube type
  for (int i = 0; i < NUM_TUBES; i++) {
    _pulsarSlotDelay[i] = 14 - TubeCathodeCount[tubes[i].Type];
  }

  _ejectionDelay = 0;
  _ejectionDelayRange = 100;
  for (int i = 0; i < NumEjections; i++) {
    ejection[i].isActive = false;
  }
}

void PulsarAnimation::handlePulsarMovement() {
  _pulsarMovementDelay--;
  if (_pulsarMovementDelay < 0) {
    if (isVisible(_pulsarIndex)) {
      _slotHelper.disableCycling(_pulsarIndex);
      _fadeHelper.setTubeBrightness(_pulsarIndex, 0);
    }

    _pulsarIndex += _pulsarDirection;
    _pulsarMovementDelay = isVisible(_pulsarIndex) ? _pulsarSpeed : _pulsarSpeed / 3;

    if (isVisible(_pulsarIndex)) {
      _slotHelper.enableCycling(_pulsarIndex, _pulsarSlotDelay[_pulsarIndex]);
      _fadeHelper.setTubeBrightness(_pulsarIndex, _maxBrightness);
    }

    if ((_pulsarDirection == Right && _pulsarIndex < -EndOffset) || (_pulsarDirection == Left && _pulsarIndex > NUM_TUBES - 1 + EndOffset)) {
      _finalEjection = true;
    }
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

    int delay = random(_ejectionDelayRange / 2, _ejectionDelayRange);

    // spawn new only if not at max active count
    if (inactiveEjectionIndex >= 0) {
      ejection[inactiveEjectionIndex].isActive = true;
      ejection[inactiveEjectionIndex].delay = 0;
      ejection[inactiveEjectionIndex].index = _pulsarIndex;
            
      if (!_finalEjection) {
        ejection[inactiveEjectionIndex].brightness = random(0.1 * _maxBrightness, 0.4 * _maxBrightness);
        ejection[inactiveEjectionIndex].direction = (random(2) == 0) ? Left : Right;
        // if pulsar is not visible then ejections travels shorter distance
        ejection[inactiveEjectionIndex].distance =  random(1, NUM_TUBES); //isVisible(_pulsarIndex) ? random(2, NUM_TUBES-1) : random(1, OffScreenOffset+1); 
        ejection[inactiveEjectionIndex].slotActive = true;
        ejection[inactiveEjectionIndex].speed = random(30, 200);
      }
      else {
        // last ejection will reset tubes to primary cathode
        ejection[inactiveEjectionIndex].brightness = _maxBrightness;
        ejection[inactiveEjectionIndex].direction = (Direction)-_pulsarDirection;
        ejection[inactiveEjectionIndex].distance = NUM_TUBES + StartOffset;
        ejection[inactiveEjectionIndex].slotActive = true;
        ejection[inactiveEjectionIndex].speed = random(70,130);
        
        delay = 0x7FFFFFFF;
      }
    }

    _ejectionDelay = delay;
  }
}

void PulsarAnimation::handleEjectionMovement() {
  for (int i = 0; i < NumEjections; i++) {
    if (ejection[i].isActive) {
      ejection[i].delay--;

      if (ejection[i].delay < 0) {
        // old tube location updates (skip first location at pulsar)
        if (ejection[i].index != _pulsarIndex && isVisible(ejection[i].index)) {
          if (ejection[i].slotActive) {
            _slotHelper.disableCycling(ejection[i].index);
          }
          
          if (!_finalEjection) {
            _fadeHelper.setTubeBrightness(ejection[i].index, 0);
          }
        }

        // update ejection location
        ejection[i].delay = ejection[i].speed;
        ejection[i].index += ejection[i].direction;
        ejection[i].distance--;
        
        if (ejection[i].distance >= 0) {
          if (isVisible(ejection[i].index)) {
            // new tube positions updates
            if (ejection[i].slotActive) {
              _slotHelper.enableCycling(ejection[i].index, 50); // TODO: varied cycle time?
            }
            else {
              _slotHelper.setRandomCathode(ejection[i].index);
            }

            _fadeHelper.setTubeBrightness(ejection[i].index, ejection[i].brightness);
          }
        }
        else {
          ejection[i].isActive = false;

          if (_finalEjection) {
            bool allInactive = true;
            for (int i = 0; i < NumEjections; i++) {
              allInactive &= !ejection[i].isActive;
            }

            if (allInactive) {
              Animation::setComplete();
            }
          }
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