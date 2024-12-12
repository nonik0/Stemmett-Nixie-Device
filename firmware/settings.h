#pragma once

#ifdef USE_PREFERENCES
#include <Preferences.h>
Preferences preferences;
#endif

#include "Animation/animation.h"

bool preferencesInitialized = false;

// animation settings
bool animationsEnabledDay[NUM_ANIMATIONS];
bool animationsEnabledNight[NUM_ANIMATIONS];
float animationDaySpeedFactor;
float animationNightSpeedFactor;
TransitionBehavior transitionBehavior = TransitionBehavior::Sequential;

// brightness settings
struct tm dayTransitionTime;
int dayBrightness;
struct tm nightTransitionTime;
int nightBrightness;

// based off current time
bool isNightMode;

// TODO: integrate?
uint16_t lightSensorThreshold = 512;

void initSettings()
{
 
#ifdef USE_PREFERENCES
  if (!preferencesInitialized)
  {
    preferences.begin("stemmett", false);
    preferencesInitialized = true;
    log_i("Initialized preferences");
  }
#endif
}

void loadSettings()
{
  log_d("Loading settings");

#ifdef USE_PREFERENCES
  initSettings();

  // use default settings until settings are modified by user
  bool isCustom = preferences.getBool("custom", false);
  if (isCustom)
  {
    log_i("Using custom settings");

    for (int i = 1; i < NUM_ANIMATIONS; i++)
    {
      String key = "a" + String(i);
      animationsEnabledDay[i] = preferences.getBool(key.c_str(), true);
    }
    for (int i = 1; i < NUM_ANIMATIONS; i++)
    {
      String key = "A" + String(i);
      animationsEnabledNight[i] = preferences.getBool(key.c_str(), true);
    }
    animationDaySpeedFactor = preferences.getFloat("ds", 1.0F);
    animationNightSpeedFactor = preferences.getFloat("ns", 1.0F);
    transitionBehavior = (TransitionBehavior)preferences.getUChar("tb", 0);
    preferences.getBytes("dtt", &dayTransitionTime,
                         sizeof(dayTransitionTime));
    dayBrightness = preferences.getUChar("db", 255);
    preferences.getBytes("ntt", &nightTransitionTime,
                         sizeof(nightTransitionTime));
    nightBrightness = preferences.getUChar("nb", 255);
  }
  else
  {
#endif
    // default settings
    log_i("Using default settings");

    for (int i = 1; i < NUM_ANIMATIONS; i++)
    {
      animationsEnabledDay[i] = true;
      animationsEnabledNight[i] = false;
    }

    animationsEnabledNight[AnimationType::Name] = true;
    animationsEnabledNight[AnimationType::BasicFade] = true;
    animationsEnabledNight[AnimationType::Bubble] = true;
    animationsEnabledNight[AnimationType::CylonScan] = true;
    animationsEnabledNight[AnimationType::Ghost] = true;
    animationsEnabledNight[AnimationType::Scan] = true;
    animationsEnabledNight[AnimationType::SlotMachine] = true;

    animationDaySpeedFactor = 1.0F;
    animationNightSpeedFactor = 0.15F;
    transitionBehavior = TransitionBehavior::Sequential;
    dayTransitionTime = {8, 0, 0, 0, 0, 0, 0, 0, 0};
    dayBrightness = 255;
    nightTransitionTime = {19, 0, 0, 0, 0, 0, 0, 0, 0};
    nightBrightness = 32;
#ifdef USE_PREFERENCES
  }
#endif
}

void saveSettings()
{
#ifdef USE_PREFERENCES
  log_d("Saving settings");

  initSettings();

  // this disables the default settings
  preferences.putBool("custom", true);

  for (int i = 1; i < NUM_ANIMATIONS; i++)
  {
    String key = "a" + String(i);
    preferences.putBool(key.c_str(), animationsEnabledDay[i]);
  }
  for (int i = 1; i < NUM_ANIMATIONS; i++)
  {
    String key = "A" + String(i);
    preferences.putBool(key.c_str(), animationsEnabledNight[i]);
  }
  preferences.putFloat("ds", animationDaySpeedFactor);
  preferences.putFloat("ns", animationNightSpeedFactor);
  preferences.putUChar("tb", transitionBehavior);
  preferences.putBytes("dtt", &dayTransitionTime,
                       sizeof(dayTransitionTime));
  preferences.putUChar("db", dayBrightness);
  preferences.putBytes("ntt", &nightTransitionTime,
                       sizeof(nightTransitionTime));
  preferences.putUChar("nb", nightBrightness);

  log_i("Saved settings");
#else
  log_w("Preferences not enabled, settings not saved");
#endif
}