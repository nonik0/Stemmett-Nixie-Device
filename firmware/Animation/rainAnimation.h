#pragma once

#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "Helpers/slotHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  int initialBrightness;
  int fadeDuration;
} Raindrop;

class RainAnimation : public Animation {
  private:
    //int _activeBubbleCount;
    int _activePhaseDuration;
    int _newRaindropCooldown;
    int _baseCooldown;
    Raindrop _raindrops[NUM_TUBES];
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
