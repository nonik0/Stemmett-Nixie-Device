#pragma once

#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "Helpers/slotHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  bool isFading;       // true when fading down or fading to final state
  bool justHit;        // true for one tick after hit, to wait before fading
  bool inFinalTransition;  // true when fading to primary cathode at max
  int cooldown;        // ticks until next raindrop can hit
  int maxCooldown;     // maximum ticks before forced raindrop hit
} Raindrop;

class RainAnimation : public Animation {
  private:
    static constexpr int MAX_DURATION = 25000;
    static constexpr int ACTIVE_PHASE_MIN_DURATION = 10000;
    static constexpr int ACTIVE_PHASE_MAX_DURATION = 20000;

    // Brightness after fading down (dim but visible)
    static constexpr int MIN_BRIGHTNESS = 1;

    // Fade down duration (randomized once per animation based on speed)
    static constexpr int FADE_DOWN_BASE_MIN = 300;
    static constexpr int FADE_DOWN_BASE_MAX = 600;
    static constexpr int FADE_DOWN_SPEED_FACTOR_MIN = 0;
    static constexpr int FADE_DOWN_SPEED_FACTOR_MAX = 1000;

    // Probability of raindrop hit during fade (1 in N chance per tick)
    static constexpr int HIT_PROBABILITY_DURING_FADE = 300;

    // Maximum time at min brightness before forced raindrop hit
    static constexpr int MAX_DIM_TIME_BASE_MIN = 200;
    static constexpr int MAX_DIM_TIME_BASE_MAX = 500;
    static constexpr int MAX_DIM_TIME_SPEED_FACTOR_MIN = 0;
    static constexpr int MAX_DIM_TIME_SPEED_FACTOR_MAX = 500;

    // Initial raindrop stagger range
    static constexpr int INITIAL_STAGGER_MAX = 1500;

    int _activePhaseDuration;
    int _fadeDownDuration;  // Fixed duration for this animation instance
    Raindrop _raindrops[NUM_TUBES];
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;

  private:
    void hitRaindrop(int tubeIndex, bool isActivePhase);
};
