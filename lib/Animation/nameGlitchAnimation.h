#include <Arduino.h>

#include "animationBase.h"
#include "tubes.h"

// typedef struct {
//   uint8_t flickerCount;
//   bool isComplete;
// } Glitch;

class NameGlitchAnimation : public Animation {
  private:
    int _flickerDelay;
  public:
    void initialize(Tube tubes[NUM_TUBES]) override {
      Serial.println("NameGlitchAnimation::initialize");
      Animation::setDuration(2000);
      _flickerDelay = 0;
    }

    TickResult handleTick(Tube tubes[NUM_TUBES]) override {
      // TODO: Stella-specific impl
      Animation::handleTick(tubes);

      bool update = false;

      _flickerDelay--;

      if (_flickerDelay < 0) {
        if (tubes[3].ActiveCathode == tubes[3].PrimaryCathode) {
          tubes[3].ActiveCathode = IN7A_M;
          _flickerDelay = 2 + random(0, 13);
        }
        else {
          tubes[3].ActiveCathode = tubes[3].PrimaryCathode;
          _flickerDelay = random(0, 500);
        }

        update = true;
      }

      return {update, false};
    }
};