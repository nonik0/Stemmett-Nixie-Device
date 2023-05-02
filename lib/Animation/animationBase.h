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
