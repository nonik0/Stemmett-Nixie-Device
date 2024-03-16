#include <Arduino.h>

#include "animationBase.h"
#include "tubes.h"

class NameAnimation : public Animation {
  private:
    bool  _firstTick;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) {
      Serial.printf("NameAnimation::initialize(%d)", maxBrightness);
      Animation::initialize(tubes, maxBrightness);
      Animation::setDuration(2000);
      _firstTick = true;
    }

    TickResult handleTick(Tube tubes[NUM_TUBES]) {
      Animation::handleTick(tubes);

      if (_firstTick) {
        for (int i = 0; i < NUM_TUBES; i++) {
          tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
          tubes[i].Brightness = _maxBrightness;
        }

        _firstTick = false;
        return {true,true};
      }

      return {false,false};
    }
};
