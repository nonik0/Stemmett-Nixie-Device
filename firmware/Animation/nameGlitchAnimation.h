#include <Arduino.h>

#include "animationBase.h"
#include "tubes.h"

typedef struct {
  uint8_t cathode;
  int delay;
  int onDelayRange;
  int offDelayRange;
} Glitch;


#if defined(STELLA)
Glitch glitch[NUM_TUBES] = {
            {IN7_V, 0, 0, 0},
          {IN4_1, 0, 0, 0},
        {IN4_1, 0, 0, 0},
      {IN7A_M, 0, 0, 0},
    {IN7_Minus, 0, 0, 0},
  {IN7B_Hz, 0, 0, 0},
};
#elif defined(EMMETT)
Glitch glitch[NUM_TUBES] = {
            {IN7_Minus, 0, 0, 0},
          {IN7_Minus, 0, 0, 0},
        {IN7_M, 0, 0, 0},
      {IN7_M, 0, 0, 0},
    {IN7_M, 0, 0, 0},
  {IN7_M, 0, 0, 0},
};
#endif

class NameGlitchAnimation : public Animation {
  private:
    int _flickerDelay;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) override {
      log_d("NameGlitchAnimation::initialize");
      Animation::initialize(tubes, maxBrightness, speedFactor);
      Animation::setDuration(10000);

      for (int i = 0; i < NUM_TUBES; i++) {
        if (glitch[i].cathode != 0xFF) {
          int minDelay = 200 * (1 - speedFactor); // slowest: 200, fastest: 0
          int maxDelay = 1000 + 2000 * (1 - speedFactor); // slowest: 3000, fastest: 1000
          glitch[i].delay = random(minDelay, maxDelay);

          int minOffDelay = 300 + 600 * (1 - speedFactor); // slowest: 600, fastest: 300
          int maxOffDelay = 700 + 1400 * (1 - speedFactor); // slowest: 2100, fastest: 700         
          glitch[i].offDelayRange = random(minOffDelay, maxOffDelay);

          int minOnDelay = 10 + 20 * (1 - speedFactor); // slowest: 30, fastest: 10
          int maxOnDelay = 20 + 70 * (1 - speedFactor); // slowest: 90, fastest: 20
          glitch[i].onDelayRange = random(minOnDelay, maxOnDelay);
        }
      }
    }

    TickResult handleTick(Tube tubes[NUM_TUBES]) override {
      // TODO: Stella-specific impl
      Animation::handleTick(tubes);

      bool update = false;

      for (int i = 0; i < NUM_TUBES; i++) {
        if (glitch[i].cathode != 0xFF) {
          glitch[i].delay--;

          if (glitch[i].delay < 0) {
            if (tubes[i].ActiveCathode == tubes[i].PrimaryCathode) {
              tubes[i].ActiveCathode = glitch[i].cathode;
              glitch[i].delay = random(2, glitch[i].onDelayRange);
            }
            else {
              tubes[i].ActiveCathode = tubes[i].PrimaryCathode;
              glitch[i].delay = random(0, glitch[i].offDelayRange);
            }

            update = true;
          }
        }
      }

      return {update, false};
    }
};