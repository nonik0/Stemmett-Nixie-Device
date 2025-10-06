#include <Arduino.h>

#include "ghostAnimation.h"

void GhostAnimation::initialize(Tube tubes[NUM_TUBES], int maxBrightness, float speedFactor)
{
  log_d("GhostAnimation::initialize");
  Animation::initialize(tubes, maxBrightness, speedFactor);
  Animation::setDuration(MAX_DURATION);

  _activeGhostCount = 0;
  _activePhaseDuration = random(ACTIVE_PHASE_MIN_DURATION, ACTIVE_PHASE_MAX_DURATION);

  // All tubes start at maxBrightness, no ghosts active yet
  // Stagger initial cooldowns so tubes don't all start at once
  for (int i = 0; i < NUM_TUBES; i++)
  {
    _ghost[i].isActive = false;
    _ghost[i].isFirstGhost = true;  // First ghost on each tube will skip fade up
    _ghost[i].cooldown = random(0, 500) * i / NUM_TUBES;  // Stagger by up to 500ms
  }
}

TickResult GhostAnimation::handleTick(Tube tubes[NUM_TUBES])
{
  Animation::handleTick(tubes);

  bool update = _fadeHelper.handleTick(tubes);

  _activePhaseDuration--;

  bool isActivePhase = _activePhaseDuration > 0;

  // Check each tube for ghost spawning and fade completion
  for (int i = 0; i < NUM_TUBES; i++)
  {
    // Decrement cooldown
    if (_ghost[i].cooldown > 0)
    {
      _ghost[i].cooldown--;
    }

    // Spawn new ghost if cooldown complete and tube inactive
    // During active phase: spawn normally
    // After active phase: only spawn if tube not at maxBrightness (for final fade up)
    if (!_ghost[i].isActive && _ghost[i].cooldown <= 0)
    {
      if (isActivePhase || tubes[i].Brightness < _maxBrightness)
      {
        startGhost(i);
      }
    }

    // Handle ghost fade completion
    if (_ghost[i].isActive && _fadeHelper.isComplete(i))
    {
      handleGhostFadeComplete(tubes, i, isActivePhase);
    }
  }

  // Check if animation complete (after active phase, all tubes at max)
  if (!isActivePhase)
  {
    int tubesAtMax = 0;
    for (int i = 0; i < NUM_TUBES; i++)
    {
      if (tubes[i].Brightness == _maxBrightness)
      {
        tubesAtMax++;
      }
    }

    if (tubesAtMax == NUM_TUBES)
    {
      setComplete();
      return {false, update};
    }
  }

  return {false, update};
}

void GhostAnimation::startGhost(int tubeIndex)
{
  bool isActivePhase = _activePhaseDuration > 0;
  int minFadeDuration = FADE_DURATION_BASE_MIN + FADE_DURATION_SPEED_FACTOR_MIN * (1 - _speedFactor);  // slowest: 600, fastest: 200
  int maxFadeDuration = FADE_DURATION_BASE_MAX + FADE_DURATION_SPEED_FACTOR_MAX * (1 - _speedFactor);  // slowest: 2000, fastest: 800
  int fadeDuration = random(minFadeDuration, maxFadeDuration);

  _ghost[tubeIndex].isActive = true;
  _ghost[tubeIndex].fadeDuration = fadeDuration;
  _ghost[tubeIndex].targetBrightnessLow = random(GHOST_TARGET_BRIGHTNESS_LOW_MIN, GHOST_TARGET_BRIGHTNESS_LOW_MAX);
  _ghost[tubeIndex].targetBrightnessHigh = random(GHOST_TARGET_BRIGHTNESS_MIN, _maxBrightness);

  // After active phase: fade directly to maxBrightness
  if (!isActivePhase)
  {
    _ghost[tubeIndex].isFadingUp = true;
    log_d("Start final ghost %d: fade to maxBrightness %d (fadeDuration %d)", tubeIndex, _maxBrightness, fadeDuration);
    _fadeHelper.setTubeFade(tubeIndex, _maxBrightness, fadeDuration);
  }
  // First ghost on each tube starts by fading down from maxBrightness
  else if (_ghost[tubeIndex].isFirstGhost)
  {
    _ghost[tubeIndex].isFirstGhost = false;
    _ghost[tubeIndex].isFadingUp = false;
    log_d("Start first ghost %d: fade down to %d (fadeDuration %d)", tubeIndex, _ghost[tubeIndex].targetBrightnessLow, fadeDuration);
    _fadeHelper.setTubeFade(tubeIndex, _ghost[tubeIndex].targetBrightnessLow, fadeDuration);
  }
  // Subsequent ghosts start by fading up
  else
  {
    _ghost[tubeIndex].isFadingUp = true;
    log_d("Start ghost %d: fade up to %d (fadeDuration %d)", tubeIndex, _ghost[tubeIndex].targetBrightnessHigh, fadeDuration);
    _fadeHelper.setTubeFade(tubeIndex, _ghost[tubeIndex].targetBrightnessHigh, fadeDuration);
  }

  _activeGhostCount++;
}

void GhostAnimation::handleGhostFadeComplete(Tube tubes[NUM_TUBES], int tubeIndex, bool isActivePhase)
{
  Ghost_t& ghost = _ghost[tubeIndex];

  // Ghost finished fading up
  if (ghost.isFadingUp)
  {
    // If active phase is over and we reached maxBrightness, ghost is done
    if (!isActivePhase && tubes[tubeIndex].Brightness == _maxBrightness)
    {
      log_d("Ghost %d complete at maxBrightness (%d)", tubeIndex, _maxBrightness);
      ghost.isActive = false;
      _activeGhostCount--;
      return;
    }

    // Otherwise, fade back down
    ghost.isFadingUp = false;
    log_d("Ghost %d fade down to %d (fadeDuration %d)", tubeIndex, ghost.targetBrightnessLow, ghost.fadeDuration);
    _fadeHelper.setTubeFade(tubeIndex, ghost.targetBrightnessLow, ghost.fadeDuration);
  }
  // Ghost finished fading down
  else
  {
    // Deactivate ghost and set cooldown for next ghost
    ghost.isActive = false;
    _activeGhostCount--;

    // If active phase is over, do final fade to maxBrightness immediately (no cooldown)
    if (!isActivePhase)
    {
      ghost.cooldown = 0;
    }
    // Otherwise, set random cooldown before next ghost
    else
    {
      int minCooldown = GHOST_COOLDOWN_BASE_MIN + GHOST_COOLDOWN_SPEED_FACTOR_MIN * (1 - _speedFactor); // slowest: 200, fastest: 60
      int maxCooldown = GHOST_COOLDOWN_BASE_MAX + GHOST_COOLDOWN_SPEED_FACTOR_MAX * (1 - _speedFactor); // slowest: 1400, fastest: 800
      ghost.cooldown = random(minCooldown, maxCooldown);
      log_d("Ghost %d deactivated, cooldown %d", tubeIndex, ghost.cooldown);
    }
  }
}