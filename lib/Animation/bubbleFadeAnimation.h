#include "animationBase.h"
#include "fadeHelper.h"
#include "tubes.h"

typedef struct {
  bool isActive;
  // int targetHigh;
  // int targetLow;
  int targetBrightness;
  int fadeDuration;
} Bubble;

class BubbleFadeAnimation : public Animation {
  private:
    int _activeBubbleCount;
    int _activeBubbleDuration;
    int _newBubbleCooldown;
    Bubble _bubbles[NUM_TUBES];
    FadeHelper _fadeHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
