#include "tubeTestAnimation.h"

void TubeTestAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("TubeTestAnimation::initialize");
  Animation::setDuration(TEST_DURATION_MS);
  Animation::initialize(tubes, maxBrightness, speedFactor);

  _tickCount = 0;

  const int ticksPerMs = 1000 / REFRESH_RATE_US;

  for (int i = 0; i < NUM_TUBES; i++) {
    TubeType tubeType = tubes[i].Type;

    // ticks per cathode (add 1 to cathode count to make first cathode display 2x duration)
    int cathodeCount = TubeCathodeCount[tubeType];
    _cathodeDisplayTicks[i] = (TEST_DURATION_MS / (cathodeCount + 1)) * ticksPerMs;

    // start at index 0
    _currentCathodeIndex[i] = 0;
    tubes[i].ActiveCathode = TubeCathodes[tubeType][0];
    tubes[i].Brightness = maxBrightness;

    log_d("Tube %d: Type=%d, Cathodes=%d, Ticks per cathode=%d",
          i, tubeType, cathodeCount, _cathodeDisplayTicks[i]);
  }
}

TickResult TubeTestAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  bool cathodeUpdate = false;
  bool brightnessUpdate = false;

  for (int i = 0; i < NUM_TUBES; i++) {
    TubeType tubeType = tubes[i].Type;
    int cathodeCount = TubeCathodeCount[tubeType];

    // first cathode is first two cycles for double time
    int targetCathodeIndex = max((_tickCount / _cathodeDisplayTicks[i]) - 1, 0);

    if (targetCathodeIndex != _currentCathodeIndex[i] && targetCathodeIndex < cathodeCount) {
      _currentCathodeIndex[i] = targetCathodeIndex;
      tubes[i].ActiveCathode = TubeCathodes[tubeType][targetCathodeIndex];
      cathodeUpdate = true;

      log_d("Tube %d: Advancing to cathode index %d (cathode value %d)",
            i, targetCathodeIndex, tubes[i].ActiveCathode);
    }
  }

  // set initial cathodes on first tick
  if (_tickCount == 0) {
    cathodeUpdate = true;
    brightnessUpdate = true;
  }

  _tickCount++;

  // Check if animation is complete
  if (_tickCount * REFRESH_RATE_US >= TEST_DURATION_MS * 1000) {
    log_d("TubeTestAnimation complete after %d ticks", _tickCount);
    setComplete();
  }

  return {cathodeUpdate, brightnessUpdate};
}
