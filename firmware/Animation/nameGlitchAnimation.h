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
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override {
      log_i("NameGlitchAnimation::initialize");
      Animation::initialize(tubes, maxBrightness);
      Animation::setDuration(10000);

      for (int i = 0; i < NUM_TUBES; i++) {
        if (glitch[i].cathode != 0xFF) {
          glitch[i].delay = random(0, 1000);
          glitch[i].offDelayRange = random(300, 700);
          glitch[i].onDelayRange = random(10, 20);
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