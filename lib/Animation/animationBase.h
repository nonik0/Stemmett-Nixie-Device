#pragma once

#include "tubes.h"

typedef struct {
  bool CathodeUpdate;
  bool BrightnessUpdate;
} TickResult;

class Animation {
 private:
  int _durationMs = 3600;

 public:
  virtual ~Animation() = default;
  virtual void initialize(Tube tubes[NUM_TUBES]) = 0;
  virtual TickResult handleTick(Tube tubes[NUM_TUBES]) = 0;
  bool isComplete();
  void setDuration(int durationMs);
};

// void Animation::setDuration(int durationMs) { _durationMs = durationMs; }

// TickResult Animation::handleTick(Tube tubes[NUM_TUBES]) {
//   _durationMs--;
//   return {false, false};
// }

// bool Animation::isComplete() {
//   return _durationMs < 0;
// }
