#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  int targetBrightnessLow;
  int targetBrightnessHigh;
  int fadeDuration;
} Ghost_t;

class GhostAnimation : public Animation {
  private:
    int _activeGhostCount;
    int _activePhaseDuration;
    int _newGhostCooldown;
    Ghost_t _ghost[NUM_TUBES];
    FadeHelper _fadeHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
