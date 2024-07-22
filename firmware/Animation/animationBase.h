#pragma once

#include "tubes.h"

// TODO: move somewhere else?
typedef enum { Left = 1, Right = -1 } Direction;

typedef struct {
  bool CathodeUpdate;
  bool BrightnessUpdate;
} TickResult;

class Animation {
 private:
  int _durationMs = 3600;
 protected:
  volatile int _maxBrightness;
  volatile float _speedFactor;
 public:
  // TODO: can do ctor instead of initialize impl?
  virtual ~Animation() = default;
  virtual void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) = 0;
  virtual TickResult handleTick(Tube tubes[NUM_TUBES]) = 0;
  bool isComplete();
  void setComplete();
  void setBrightness(int brightness);
  void setDuration(int durationMs);
  void setSpeed(float speed);
};
