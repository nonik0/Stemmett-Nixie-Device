#include "animationBase.h"
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

typedef struct {
  uint8_t targetBrightness;
  int stepDelay;
  uint8_t stepsLeft;
} TubeFadeState;

class FadeAnimation : public Animation {
  private:
    const int FadeStepDelay = 32; // TODO: needs to be different for different fade durations
    FadeProgression _fadeProgression;
    int* getRandomInitialDelays(FadePattern fadeType, int fadeMultiplier);
    int* getRandomTubeOrder();
    void setNextProgression();
    void setRandomFadePattern(int targetBrightness, int initialDelay = 0);
    void setTubeFade(int tubeIndex, int targetBrightness, int durationMs, int initialDelay);
  protected:
    TubeFadeState _tubeFadeState[NUM_TUBES]; // TODO: FadeHandler
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
