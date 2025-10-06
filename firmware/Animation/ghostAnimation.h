#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  bool isFirstGhost;  // true if tube hasn't had a ghost yet (skip fade up)
  bool isFadingUp;    // true when fading to high, false when fading to low
  int targetBrightnessLow;
  int targetBrightnessHigh;
  int fadeDuration;
  int cooldown;       // ticks until next ghost can start on this tube
} Ghost_t;

class GhostAnimation : public Animation {
  private:
    static constexpr int MAX_DURATION = 25000;
    static constexpr int ACTIVE_PHASE_MIN_DURATION = 10000;
    static constexpr int ACTIVE_PHASE_MAX_DURATION = 20000;
    static constexpr int ACTIVE_PHASE_GHOST_COUNT = 5;

    static constexpr int GHOST_TARGET_BRIGHTNESS_MIN = 20;
    static constexpr int GHOST_TARGET_BRIGHTNESS_LOW_MIN = 0;
    static constexpr int GHOST_TARGET_BRIGHTNESS_LOW_MAX = 30;

    static constexpr int FADE_DURATION_BASE_MIN = 200;
    static constexpr int FADE_DURATION_BASE_MAX = 800;
    static constexpr int FADE_DURATION_SPEED_FACTOR_MIN = 400;
    static constexpr int FADE_DURATION_SPEED_FACTOR_MAX = 1200;

    static constexpr int GHOST_COOLDOWN_BASE_MIN = 60;
    static constexpr int GHOST_COOLDOWN_BASE_MAX = 800;
    static constexpr int GHOST_COOLDOWN_SPEED_FACTOR_MIN = 140;
    static constexpr int GHOST_COOLDOWN_SPEED_FACTOR_MAX = 600;

    int _activeGhostCount;
    int _activePhaseDuration;
    Ghost_t _ghost[NUM_TUBES];
    FadeHelper _fadeHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;

  private:
    void startGhost(int tubeIndex);
    void handleGhostFadeComplete(Tube tubes[NUM_TUBES], int tubeIndex, bool isActivePhase);
};
