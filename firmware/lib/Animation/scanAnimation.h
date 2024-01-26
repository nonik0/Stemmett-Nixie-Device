#include <Arduino.h>

#include "animationBase.h"
#include "fadeHelper.h"
#include "slotHelper.h"
#include "tubes.h"

class ScanAnimation : public Animation {
  private:
    bool _isScanComplete;
    bool _isSlotScan;
    int _scanDelay; // main timer for tick updates
    int _scanInactiveBrightness;
    int _scanIndex;
    int _scanPauseDelay; // time between scans
    int _scanShiftDelay; // time between scan moving to next tube
    int _scansLeft;

    Direction _scanDirection;
    FadeHelper _fadeHelper;
    SlotHelper _slotHelper;
  public:
    void initialize(Tube tubes[NUM_TUBES], int maxBrightness);
    TickResult handleTick(Tube tubes[NUM_TUBES]);
};
