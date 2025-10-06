#pragma once

#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "Helpers/slotHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  bool isFadingUp;     // true when fading up to max, false when waiting to pop
  bool justPopped;     // true immediately after popping
  bool doneFinalPop;   // true after final pop to primary cathode
  int cooldown;        // ticks until bubble pops or starts fading
  int fadeDuration;    // duration of current fade up
} Bubble_t;

class BubbleAnimation : public Animation {
  private:
    static constexpr int MAX_DURATION = 25000;
    static constexpr int ACTIVE_PHASE_MIN_DURATION = 10000;
    static constexpr int ACTIVE_PHASE_MAX_DURATION = 20000;

    // Brightness after pop (dim but visible)
    static constexpr int POP_BRIGHTNESS = 1;

    // Fade up duration range (affected by speed factor)
    static constexpr int FADE_UP_BASE_MIN = 300;    // fastest at max speed
    static constexpr int FADE_UP_BASE_MAX = 500;    // slowest at max speed
    static constexpr int FADE_UP_SPEED_FACTOR_MIN = 0;     // no extra time at max speed
    static constexpr int FADE_UP_SPEED_FACTOR_MAX = 600;   // extra time at min speed

    // Delay after pop before fading up (affected by speed factor)
    static constexpr int POST_POP_DELAY_BASE_MIN = 50;
    static constexpr int POST_POP_DELAY_BASE_MAX = 300;
    static constexpr int POST_POP_DELAY_SPEED_FACTOR_MIN = 0;
    static constexpr int POST_POP_DELAY_SPEED_FACTOR_MAX = 300;

    // Initial pop stagger range
    static constexpr int INITIAL_POP_STAGGER_MAX = 1500;

    int _activePhaseDuration;
    Bubble_t _bubble[NUM_TUBES];
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;

  private:
    void popBubble(int tubeIndex, bool isActivePhase);
    void startFadeUp(int tubeIndex);
};
