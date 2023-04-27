#include "animationBase.h"
#include "tubes.h"

typedef struct {
  uint8_t flickerCount;
  bool isComplete;
} Glitch;

class NameGlitchAnimation : public Animation {
  private:
    bool _firstTick;
  public:
    void initialize(Tube tubes[NUM_TUBES]) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
