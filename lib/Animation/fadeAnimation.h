#include "animationBase.h"
#include "tubes.h"

typedef enum {
  FadeLeft,
  FadeRight,
  FadeRandom
} FadeType;

typedef enum {
  FadeOut = 0,
  FadeIn = 1,
  FadeComplete = 2
} FadeProgression;

typedef struct {
  uint8_t targetBrightness;
  int step; // function pointer?
  int delayReset;
  int delay;
  bool isComplete;
} TubeFadeState;

class FadeAnimation : public Animation {
  private:
    FadeType _fadeInType;
    FadeType _fadeOutType;
    FadeProgression _fadeProgression;
    TubeFadeState _tubeFadeState[NUM_TUBES]; // TODO: FadeHandler
    void setFade(Tube tubes[NUM_TUBES], int targetBrightness);
  public:
    void initialize(Tube tubes[NUM_TUBES]) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
