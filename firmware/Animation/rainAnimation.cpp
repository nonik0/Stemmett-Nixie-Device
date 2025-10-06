#include <Arduino.h>

#include "rainAnimation.h"

void RainAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("RainAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(MAX_DURATION);

  _activePhaseDuration = random(ACTIVE_PHASE_MIN_DURATION, ACTIVE_PHASE_MAX_DURATION);

  // Set fade down duration once for entire animation (based on speed factor)
  int minFadeDuration = FADE_DOWN_BASE_MIN + FADE_DOWN_SPEED_FACTOR_MIN * (1 - _speedFactor);
  int maxFadeDuration = FADE_DOWN_BASE_MAX + FADE_DOWN_SPEED_FACTOR_MAX * (1 - _speedFactor);
  _fadeDownDuration = random(minFadeDuration, maxFadeDuration);
  log_d("Rain fade duration: %d", _fadeDownDuration);

  // All tubes start at maxBrightness on primary cathode
  // Stagger initial raindrop hits
  for (int i = 0; i < NUM_TUBES; i++) {
    _raindrops[i].isActive = true;
    _raindrops[i].isFading = false;
    _raindrops[i].justHit = false;
    _raindrops[i].inFinalTransition = false;
    _raindrops[i].cooldown = random(0, INITIAL_STAGGER_MAX) * i / NUM_TUBES;

    int minMaxCooldown = MAX_DIM_TIME_BASE_MIN + MAX_DIM_TIME_SPEED_FACTOR_MIN * (1 - _speedFactor);
    int maxMaxCooldown = MAX_DIM_TIME_BASE_MAX + MAX_DIM_TIME_SPEED_FACTOR_MAX * (1 - _speedFactor);
    _raindrops[i].maxCooldown = random(minMaxCooldown, maxMaxCooldown);
  }
}

TickResult RainAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activePhaseDuration--;
  bool isActivePhase = _activePhaseDuration > 0;

  // Process each raindrop
  for (int i = 0; i < NUM_TUBES; i++) {
    if (!_raindrops[i].isActive) continue;

    // If raindrop just hit, wait one tick then start fade down
    if (_raindrops[i].justHit) {
      _raindrops[i].justHit = false;
      _raindrops[i].isFading = true;
      _fadeHelper.setTubeFade(i, MIN_BRIGHTNESS, _fadeDownDuration);
      continue;
    }

    // Check for random hit during fade (rare) - only during active phase
    if (_raindrops[i].isFading && isActivePhase && random(HIT_PROBABILITY_DURING_FADE) == 0) {
      hitRaindrop(i, isActivePhase);
      continue;
    }

    // Raindrop finished fading down to MIN_BRIGHTNESS
    if (_raindrops[i].isFading && _fadeHelper.isComplete(i) && tubes[i].Brightness == MIN_BRIGHTNESS) {
      _raindrops[i].isFading = false;
      _raindrops[i].cooldown = _raindrops[i].maxCooldown;
      continue;
    }

    // Decrement cooldown when not fading (both active and post-active phase)
    if (!_raindrops[i].isFading && _raindrops[i].cooldown > 0) {
      _raindrops[i].cooldown--;
    }

    // Raindrop ready to hit
    if (!_raindrops[i].isFading && _raindrops[i].cooldown <= 0) {
      // During active phase: hit to random cathode
      if (isActivePhase) {
        hitRaindrop(i, true);
      }
      // After active phase: final hit to primary cathode at max, then deactivate
      else {
        _slotHelper.setPrimaryCathode(i);
        _fadeHelper.setTubeFade(i, _maxBrightness, 0);
        log_d("Raindrop %d final hit: primary cathode at max", i);
        _raindrops[i].isActive = false;
      }
      continue;
    }
  }

  // Check if animation complete (all raindrops inactive)
  if (!isActivePhase) {
    int tubesComplete = 0;
    for (int i = 0; i < NUM_TUBES; i++) {
      if (!_raindrops[i].isActive) {
        tubesComplete++;
      }
    }

    if (tubesComplete == NUM_TUBES) {
      setComplete();
      return {false, false};
    }
  }

  bool cathodeUpdate = _slotHelper.handleTick(tubes);
  bool brightnessUpdate = _fadeHelper.handleTick(tubes);

  return {cathodeUpdate, brightnessUpdate};
}

void RainAnimation::hitRaindrop(int tubeIndex, bool isActivePhase) {
  // Always hit to random cathode (only called during active phase now)
  _slotHelper.setRandomCathode(tubeIndex);
  //log_d("Raindrop hit %d: random cathode", tubeIndex);

  // Instantly set to maxBrightness
  _fadeHelper.setTubeFade(tubeIndex, _maxBrightness, 0);

  // Mark that we just hit, so we wait one tick before fading down
  _raindrops[tubeIndex].justHit = true;
}