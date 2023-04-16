#include "animationBase.h"
#include "tubes.h"

class NameAnimation : public Animation {
  private:
    bool _firstTick;
  public:
    void initialize(Tube tubes[NUM_TUBES]) override;
    TickResult handleTick(Tube tubes[NUM_TUBES]) override;
};
