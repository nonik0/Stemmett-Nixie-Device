#include <Arduino.h>

#include "bubbleAnimation.h"

void BubbleAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor) {
  log_d("BubbleAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(MAX_DURATION);

  _activePhaseDuration = random(ACTIVE_PHASE_MIN_DURATION, ACTIVE_PHASE_MAX_DURATION);

  // All tubes start at maxBrightness on primary cathode
  // Stagger initial pops so they don't all pop at once
  for (int i = 0; i < NUM_TUBES; i++) {
    _bubble[i].isActive = true;
    _bubble[i].isFadingUp = false;  // Will pop first
    _bubble[i].justPopped = false;
    _bubble[i].doneFinalPop = false;
    _bubble[i].cooldown = random(0, INITIAL_POP_STAGGER_MAX) * i / NUM_TUBES;  // Stagger pops
  }
}

TickResult BubbleAnimation::handleTick(Tube tubes[NUM_TUBES]) {
  Animation::handleTick(tubes);

  _activePhaseDuration--;
  bool isActivePhase = _activePhaseDuration > 0;

  // Process each bubble
  for (int i = 0; i < NUM_TUBES; i++) {
    if (!_bubble[i].isActive) continue;

    // If bubble just popped, set a random cooldown before fading up
    if (_bubble[i].justPopped) {
      _bubble[i].justPopped = false;

      int minDelay = POST_POP_DELAY_BASE_MIN + POST_POP_DELAY_SPEED_FACTOR_MIN * (1 - _speedFactor);
      int maxDelay = POST_POP_DELAY_BASE_MAX + POST_POP_DELAY_SPEED_FACTOR_MAX * (1 - _speedFactor);
      _bubble[i].cooldown = random(minDelay, maxDelay);
      continue;
    }

    // Check if cooldown is done and we're ready to start fading up
    if (!_bubble[i].isFadingUp && _bubble[i].cooldown <= 0 && tubes[i].Brightness == POP_BRIGHTNESS) {
      startFadeUp(i);
      continue;
    }

    // Decrement cooldown
    if (_bubble[i].cooldown > 0) {
      _bubble[i].cooldown--;
    }

    // Bubble at max brightness ready to pop
    if (!_bubble[i].isFadingUp && _bubble[i].cooldown <= 0 && tubes[i].Brightness == _maxBrightness) {
      popBubble(i, isActivePhase);
      continue;
    }

    // Bubble finished fading up
    if (_bubble[i].isFadingUp && _fadeHelper.isComplete(i) && tubes[i].Brightness == _maxBrightness) {
      // If we did the final pop to primary cathode and faded to max, deactivate
      if (_bubble[i].doneFinalPop) {
        log_d("Bubble %d complete after final fade to max", i);
        _bubble[i].isActive = false;
      }
      // Otherwise: pop again (to random during active, to primary after active)
      else {
        _bubble[i].isFadingUp = false;
        _bubble[i].cooldown = 0;
      }
    }
  }

  // Check if animation complete (all bubbles inactive and at max)
  if (!isActivePhase) {
    int tubesComplete = 0;
    for (int i = 0; i < NUM_TUBES; i++) {
      if (!_bubble[i].isActive && tubes[i].Brightness == _maxBrightness) {
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

void BubbleAnimation::popBubble(int tubeIndex, bool isActivePhase) {
  // After active phase: pop to primary cathode (final state)
  // During active phase: pop to random cathode
  if (isActivePhase) {
    _slotHelper.setRandomCathode(tubeIndex);
    //log_d("Bubble %d pop to random cathode", tubeIndex);
  } else {
    _slotHelper.setPrimaryCathode(tubeIndex);
    log_d("Bubble %d final pop to primary cathode", tubeIndex);
    _bubble[tubeIndex].doneFinalPop = true;  // Mark that we did the final pop
  }

  // Instantly dim to POP_BRIGHTNESS (not fade, instant)
  _fadeHelper.setTubeFade(tubeIndex, POP_BRIGHTNESS, 0);

  // Mark that we just popped, so we wait one tick before fading up
  _bubble[tubeIndex].justPopped = true;
}

void BubbleAnimation::startFadeUp(int tubeIndex) {
  // Calculate random fade duration based on speed factor
  int minFadeDuration = FADE_UP_BASE_MIN + FADE_UP_SPEED_FACTOR_MIN * (1 - _speedFactor);  // fastest: 100, slowest: 100
  int maxFadeDuration = FADE_UP_BASE_MAX + FADE_UP_SPEED_FACTOR_MAX * (1 - _speedFactor);  // fastest: 400, slowest: 1000
  int fadeDuration = random(minFadeDuration, maxFadeDuration);

  _bubble[tubeIndex].isFadingUp = true;
  _bubble[tubeIndex].fadeDuration = fadeDuration;

  log_d("Bubble %d fade up to %d (fadeDuration %d)", tubeIndex, _maxBrightness, fadeDuration);
  _fadeHelper.setTubeFade(tubeIndex, _maxBrightness, fadeDuration);
}