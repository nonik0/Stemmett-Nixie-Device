#include "animationBase.h"
#include "Helpers/fadeHelper.h"
#include "tubes.h"

typedef enum {
  FadeLeft = 0,
  FadeRight = 1,
  FadeRandom = 2,
  FadePatternCount = 3
} FadePattern;

typedef enum {
  FadeStart = 0,
  FadeOut = 1,
  FadeIn = 2,
  FadeComplete = 3
} FadeProgression;

class BasicFadeAnimation : public Animation {
  private:
    FadeProgression _fadeProgression;
    FadeHelper _fadeHelper;
    int* getRandomInitialDelays(FadePattern fadeType, int fadeMultiplier);
    int* getRandomTubeOrder();
    void setNextProgression();
    void setRandomFadePattern(int targetBrightness, int initialDelay = 0);
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
