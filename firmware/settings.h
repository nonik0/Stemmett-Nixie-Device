#pragma once

#include <Preferences.h>

#include "Animation/animation.h"

bool preferencesInitialized = false;
Preferences preferences;

// animation settings
bool animationsEnabled[NUM_ANIMATIONS];
TransitionBehavior transitionBehavior = TransitionBehavior::Sequential;

// brightness settings
int brightness;
struct tm dayTransitionTime;
int dayBrightness;
struct tm nightTransitionTime;
int nightBrightness;

void initSettings() {
  if (!preferencesInitialized) {
    preferences.begin("stemmett", false);
    preferencesInitialized = true;
    log_i("Initialized preferences");
  }
}

void loadSettings() {
  initSettings();

  for (int i = 1; i < NUM_ANIMATIONS; i++) {
    String key = "a" + String(i);
    animationsEnabled[i] = preferences.getBool(key.c_str(), true);
  }
  transitionBehavior = (TransitionBehavior)preferences.getUChar("tb", 0);
  brightness = preferences.getUChar("b", 255);
  preferences.getBytes("dtt", &dayTransitionTime,
                       sizeof(dayTransitionTime));
  dayBrightness = preferences.getUChar("db", 255);
  preferences.getBytes("ntt", &nightTransitionTime,
                       sizeof(nightTransitionTime));
  nightBrightness = preferences.getUChar("nb", 255);

  log_i("Loaded settings");
}

void saveSettings() {
  initSettings();

  for (int i = 1; i < NUM_ANIMATIONS; i++) {
    String key = "a" + String(i);
    preferences.putBool(key.c_str(), animationsEnabled[i]);
  }
  preferences.putUChar("tb", transitionBehavior);
  preferences.putUChar("b", brightness);
  preferences.putBytes("dtt", &dayTransitionTime,
                       sizeof(dayTransitionTime));
  preferences.putUChar("db", dayBrightness);
  preferences.putBytes("ntt", &nightTransitionTime,
                       sizeof(nightTransitionTime));
  preferences.putUChar("nb", nightBrightness);

  log_i("Saved settings");
}