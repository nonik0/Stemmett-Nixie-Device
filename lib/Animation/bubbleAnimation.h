#pragma once

#include "animationBase.h"
#include "fadeHelper.h"
#include "slotHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  int fadeDuration;
} Bubble;

class BubbleAnimation : public Animation {
  private:
    int _activePhaseDuration;
    int _newBubbleCooldown;
    int _baseCooldown;
    Bubble _bubble[NUM_TUBES];
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
