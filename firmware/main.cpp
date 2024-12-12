#include "Animation/animation.h"
#include "nixieDriver.h"
#include "rtc.h"
#include "tubeConfiguration.h"
#include "tubes.h"
#include "wifiServices.h"

#define LIGHT_SENSOR_PIN 8

// animation variables
Animation *animations[NUM_ANIMATIONS];
Animation *curAnimation;
int curAnimationIndex = 0;
int curAnimationIndexSeq = 0;
hw_timer_t *refreshTimer;
volatile bool refreshTick = false;

WifiServices wifiServices;

// modified by rest API and displaySettingsTask
volatile int brightness = 150;
volatile float speedFactor = 1;

// timers for day/night transition
unsigned long dayNightTransitionLastUpdateMillis = 0;
unsigned long dayNightTranstionUpdateInteralMs = 0;

// light sensor variables
const unsigned long LightSensorReadIntervalMs = 1000;
const unsigned long LightSensorMinModeChangeIntervalMs = 10 * 1000;
const int LightSensorReadings = 20;
uint16_t lightSensorReadings[LightSensorReadings];
uint16_t lightSensorAverageReading = lightSensorThreshold;
unsigned long lightSensorLastReadMillis = 0;
unsigned long lightSensorLastModeChangeMillis = 0;

void IRAM_ATTR refreshTimerCallback()
{
  refreshTick = true;
}

bool ableToTransition()
{
  // check if any animations are enabled other than the name animation
  for (int i = 1; i < NUM_ANIMATIONS; i++)
  {
    if (!isNightMode && animationsEnabledDay[i])
      return true;
    else if (isNightMode && animationsEnabledNight[i])
      return true;
  }

  log_d("No animations enabled for current mode, not able to transition");
  return false;
}

bool isEnabled(int animationIndex)
{
  if (animationIndex < 0 || animationIndex >= NUM_ANIMATIONS)
  {
    log_e("Invalid animation index: %d", animationIndex);
    return false;
  }

  return isNightMode ? animationsEnabledNight[animationIndex] : animationsEnabledDay[animationIndex];
}

void instantiateAllAnimations()
{
  // initialize animations
  animations[AnimationType::BasicFade] = new BasicFadeAnimation();
  animations[AnimationType::Bubble] = new BubbleAnimation();
  animations[AnimationType::CylonScan] = new CylonScanAnimation();
  animations[AnimationType::Ghost] = new GhostAnimation();
  animations[AnimationType::Name] = new NameAnimation();
  animations[AnimationType::NameGlitch] = new NameGlitchAnimation();
  animations[AnimationType::Pulsar] = new PulsarAnimation();
  animations[AnimationType::Rain] = new RainAnimation();
  animations[AnimationType::RandomScan] = new RandomScanAnimation();
  animations[AnimationType::Scan] = new ScanAnimation();
  animations[AnimationType::SlotMachine] = new SlotMachineAnimation();
}

void initializeNextAnimation()
{
  log_d("Initializing next animation");

  int newAnimationIndex;
  if (curAnimationIndex == AnimationType::Name && ableToTransition())
  {
    do
    {
      // both exclude name animation at index 0
      if (transitionBehavior == TransitionBehavior::Sequential)
      {
        curAnimationIndexSeq = (curAnimationIndexSeq + 1) % NUM_ANIMATIONS;
        newAnimationIndex = curAnimationIndexSeq;
      }
      else if (transitionBehavior == TransitionBehavior::Random)
      {
        newAnimationIndex = random(1, NUM_ANIMATIONS);
      }
      else
      {
        log_e("Invalid transition behavior: %d", transitionBehavior);
        transitionBehavior = TransitionBehavior::Sequential;
        return;
      }

    } while (!isEnabled(newAnimationIndex) || newAnimationIndex == AnimationType::Name);

    log_d("Switching to animation %d", newAnimationIndex);
  }
  else
  {
    log_d("Switching to name animation");
    newAnimationIndex = AnimationType::Name;
  }

  curAnimationIndex = newAnimationIndex;
  curAnimation = animations[newAnimationIndex];
  curAnimation->initialize(Tubes, brightness, speedFactor);
  log_d("Initialized animation %d", newAnimationIndex);
}

void handleRefresh()
{
  if (transitionBehavior != TransitionBehavior::Sequential && transitionBehavior != TransitionBehavior::Random)
  {
    log_e("Transition behavior: %d is invalid", transitionBehavior);
    delay(10000);
    return;
  }

  if (refreshTick)
  {
    refreshTick = false;

    if (curAnimation->isComplete())
    {
      initializeNextAnimation();
    }

    TickResult result = curAnimation->handleTick(Tubes);

    if (result.CathodeUpdate)
    {
      nixieDisplay(Tubes);
    }

    if (result.BrightnessUpdate)
    {
      nixieBrightness(Tubes);
    }
  }
}

void displaySettingsTask(void *pvParameters)
{
  while (1)
  {
// optional light sensor
#ifndef USE_DS3231_RTC
    if (millis() - lightSensorLastReadMillis > LightSensorReadIntervalMs)
    {
      // read and shift in new reading
      uint16_t curReading = analogRead(LIGHT_SENSOR_PIN);
      for (int i = LightSensorReadings - 1; i > 0; i--)
      {
        lightSensorReadings[i] = lightSensorReadings[i - 1];
      }
      lightSensorReadings[0] = curReading;

      // calculate average reading
      uint16_t sum = 0;
      for (int i = 0; i < LightSensorReadings; i++)
      {
        sum += lightSensorReadings[i];
      }
      lightSensorAverageReading = sum / LightSensorReadings;

      // check if enough time has passed since last mode change before changing again
      if (millis() - lightSensorLastModeChangeMillis > LightSensorMinModeChangeIntervalMs)
      {

        if (lightSensorAverageReading < lightSensorThreshold && !isNightMode)
        {
          log_i("Light sensor reading less than threshold: %d < %d, setting to night mode", lightSensorAverageReading, lightSensorThreshold);
          isNightMode = true;
          brightness = nightBrightness;
          speedFactor = animationNightSpeedFactor;
          lightSensorLastModeChangeMillis = millis();
        }
        // don't go day mode ever at night
        else if (!isNight && lightSensorAverageReading >= lightSensorThreshold && isNightMode) 
        {
          log_i("Light sensor reading greater than threshold: %d > %d, setting to day mode", lightSensorAverageReading, lightSensorThreshold);
          isNightMode = false;
          brightness = dayBrightness;
          speedFactor = animationDaySpeedFactor;
          lightSensorLastModeChangeMillis = millis();
        }
      }

      lightSensorLastReadMillis = millis();
    }
#endif

    if (isNtpSynced && millis() - dayNightTransitionLastUpdateMillis > dayNightTranstionUpdateInteralMs)
    {
      log_d("Updating brightness and speed");

      int delaySecs, hour, minute, second;

      rtcGetTime(hour, minute, second);

      int curMins = hour * 60 + minute;
      int minsToDay = (dayTransitionTime.tm_hour * 60 + dayTransitionTime.tm_min - curMins + 1440) % 1440;
      int minsToNight = (nightTransitionTime.tm_hour * 60 + nightTransitionTime.tm_min - curMins + 1440) % 1440;

      // if minsToDay or minsToNight are 0, it means the transition time is the current time so we want the other value
      if (minsToDay == 0)
      {
        minsToDay = 1440;
      }
      if (minsToNight == 0)
      {
        minsToNight = 1440;
      }

      log_i("Current time: %02d:%02d, mins to day: %d, mins to night: %d", hour, minute, minsToDay, minsToNight);

      isNight = minsToDay < minsToNight;

      // when we transition to day we check light sensor before going to day mode
      isNightMode = isNight ? true : lightSensorAverageReading > lightSensorThreshold; 
      brightness = isNightMode ? nightBrightness : dayBrightness;
      speedFactor = isNightMode ? animationNightSpeedFactor : animationDaySpeedFactor;
      delaySecs = 60 * (isNightMode ? minsToDay : minsToNight);

      String timeOfDay = isNightMode ? "night" : "day";
      log_i("Setting brightness to %d and speed factor to %.2f (%s), next check in %dm", brightness, speedFactor, timeOfDay, delaySecs / 60);

      dayNightTranstionUpdateInteralMs = delaySecs * 1000;
      dayNightTransitionLastUpdateMillis = millis();
    }

    delay(10);
  }

  log_e("DisplaySettingsTask ended unexpectedly");
}

void setup()
{
  Serial.begin(115200);
  delay(5000);
  log_d("Starting setup");

  // fully async including wifi setup
  wifiServices.setup(DEVICE_NAME);
  wifiServices.createTask();

  loadSettings();
  nixieSetup();
  rtcSetup();

  // task to update brightness and speed
  xTaskCreate(displaySettingsTask, "DisplaySettingsTask", 8192, NULL, 10, NULL);

  refreshTimer = timerBegin(0, 80, true); // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, false);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  // start first animation
  instantiateAllAnimations();
  curAnimation = animations[AnimationType::Name];
  curAnimation->initialize(Tubes, brightness, speedFactor);

  log_i("Setup complete");
}

void loop()
{
  handleRefresh();
}
