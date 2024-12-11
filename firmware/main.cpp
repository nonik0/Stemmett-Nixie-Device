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

// timers for brightness and light sensor
unsigned long brightnessLastUpdateMillis = 0;
unsigned long brightnessUpdateIntervalMs = 0;
unsigned long lightSensorLastUpdateMillis = 0;
const unsigned long lightSensorIntervalMs = 1000;

void IRAM_ATTR refreshTimerCallback()
{
  refreshTick = true;
}

bool ableToTransition(int animationIndex = -1)
{
  if (animationIndex < 0)
  {
    // check if any animations are enabled other than the name animation
    for (int i = 1; i < NUM_ANIMATIONS; i++)
    {
      if (!isNightMode && animationsEnabledDay[i])
        return true;
      else if (isNightMode && animationsEnabledNight[i])
        return true;
    }

    log_d("No animations enabled, not able to transition");
    return false;
  }
  else
  {
    return isNightMode ? animationsEnabledNight[animationIndex] : animationsEnabledDay[animationIndex];
  }
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
        curAnimationIndex = (curAnimationIndex + 1) % NUM_ANIMATIONS;
      }
      else if (transitionBehavior == TransitionBehavior::Random)
      {
        newAnimationIndex = random(1, NUM_ANIMATIONS);
      }
    } while (!ableToTransition(newAnimationIndex) || newAnimationIndex == curAnimationIndex || newAnimationIndex == AnimationType::Name);
    log_d("Switching to animation %d", newAnimationIndex);
  }
  else
  {
    newAnimationIndex = AnimationType::Name;
  }

  curAnimationIndex = newAnimationIndex;
  curAnimation = animations[newAnimationIndex];
  curAnimation->initialize(Tubes, brightness, speedFactor);
  log_d("Initialized animation %d", newAnimationIndex);
}

void handleRefresh()
{
  if (refreshTick)
  {
    refreshTick = false;

    if (curAnimation->isComplete())
      initializeNextAnimation();

    TickResult result = curAnimation->handleTick(Tubes);

    if (result.CathodeUpdate)
    {
      nixieDisplay(Tubes);
    }

    if (result.BrightnessUpdate)
    {
      nixieBrightness(Tubes);
    }

    if (refreshTick)
    {
      log_w("Refresh took too long");
    }
  }
}

void displaySettingsTask(void *pvParameters)
{
  while (1)
  {
// optional light sensor
#ifndef USE_DS3231_RTC
    if (millis() - lightSensorLastUpdateMillis > lightSensorIntervalMs)
    {
      uint16_t reading = analogRead(LIGHT_SENSOR_PIN);

      if (reading < lightSensorThreshold && !isNightMode)
      {
        log_i("Light sensor reading: %d, setting to night mode", reading);
        brightness = nightBrightness;
        speedFactor = animationNightSpeedFactor;
        curAnimation->setBrightness(brightness);
        curAnimation->setSpeed(speedFactor);
      }
      else if (reading >= lightSensorThreshold && isNightMode)
      {
        log_i("Light sensor reading: %d, setting to day mode", reading);
        brightness = dayBrightness;
        speedFactor = animationDaySpeedFactor;
        curAnimation->setBrightness(brightness);
        curAnimation->setSpeed(speedFactor);
      }

      lightSensorLastUpdateMillis = millis();
    }
#endif

    if (isNtpSynced && millis() - brightnessLastUpdateMillis > brightnessUpdateIntervalMs)
    {
      log_d("Updating brightness and animation speed");

      int delaySecs, hour, minute, second;

      rtcGetTime(hour, minute, second);

      int curMins = hour * 60 + minute;
      int minsToDay = (dayTransitionTime.tm_hour * 60 + dayTransitionTime.tm_min - curMins + 1440) % 1440;
      int minsToNight = (nightTransitionTime.tm_hour * 60 + nightTransitionTime.tm_min - curMins + 1440) % 1440;

      // if minsToDay or minsToNight are 0, it means the transition time is the current time so we want the other value
      if (minsToDay == 0)
        minsToDay = 1440;
      if (minsToNight == 0)
        minsToNight = 1440;

      log_i("Current time: %02d:%02d, mins to day: %d, mins to night: %d", hour, minute, minsToDay, minsToNight);

      isNightMode = minsToDay < minsToNight;
      brightness = isNightMode ? nightBrightness : dayBrightness;
      speedFactor = isNightMode ? animationNightSpeedFactor : animationDaySpeedFactor;
      delaySecs = 60 * (isNightMode ? minsToDay : minsToNight);

      String timeOfDay = isNightMode ? "night" : "day";
      log_i("Setting brightness to %d and speed factor to %.2f (%s), next check in %dm", brightness, speedFactor, timeOfDay, delaySecs / 60);

      curAnimation->setBrightness(brightness);
      curAnimation->setSpeed(speedFactor);

      brightnessUpdateIntervalMs = delaySecs * 1000;
      brightnessLastUpdateMillis = millis();
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

  nixieSetup();
  rtcSetup();
  loadSettings();

  wifiServices.setup(DEVICE_NAME);
  wifiServices.createTask();

  refreshTimer = timerBegin(0, 80, true); // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, false);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  // start first animation
  instantiateAllAnimations();
  curAnimation = animations[AnimationType::Name];
  curAnimation->initialize(Tubes, brightness, speedFactor);

  // task to update brightness and speed
  xTaskCreate(displaySettingsTask, "DisplaySettingsTask", 8192, NULL, 10, NULL);

  log_i("Setup complete");
}

void loop()
{
  handleRefresh();
}
