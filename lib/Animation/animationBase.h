#pragma once

#include "tubes.h"

// TODO: move somewhere else?
typedef enum { Left, Right, DirectionCount } Direction;

typedef struct {
  bool CathodeUpdate;
  bool BrightnessUpdate;
} TickResult;

class Animation {
 private:
  int _durationMs = 3600;
 protected:
  int _maxBrightness;
 public:
  // TODO: can do ctor instead of initialize impl?
  virtual ~Animation() = default;
  virtual void initialize(Tube tubes[NUM_TUBES], int maxBrightness) = 0;
  virtual TickResult handleTick(Tube tubes[NUM_TUBES]) = 0;
  bool isComplete();
  void setDuration(int durationMs);
};
