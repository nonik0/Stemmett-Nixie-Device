#include "Animation/animation.h"
#include "nixieDriver.h"
#include "rest.h"
#include "rtc.h"
#include "services.h"
#include "tubeConfiguration.h"
#include "tubes.h"

Animation *animations[NUM_ANIMATIONS];
Animation *curAnimation;
int curAnimationIndex = 0;

hw_timer_t *refreshTimer;
volatile bool refreshTick = false;

int brightness = 150;
int brightnessLastUpdateMillis = 0;
int brightnessDelayMs = 0;

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
      if (!isNight && animationsEnabledDay[i])
        return true;
      else if (isNight && animationsEnabledNight[i])
        return true;
      return false;
    }
  }
  else
  {
    return isNight ? animationsEnabledNight[animationIndex] : animationsEnabledDay[animationIndex];
  }
}

void initializeAllAnimations()
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

  // enable animations
  for (int i = 0; i < NUM_ANIMATIONS; i++)
  {
    animationsEnabledDay[i] = true;
  }
}

void initializeNewAnimation()
{
  log_d("Initializing new animation");

  int newAnimationIndex;
  if (curAnimationIndex == AnimationType::Name && ableToTransition())
  {
    do
    {
      if (transitionBehavior == TransitionBehavior::Sequential)
        newAnimationIndex = 1 + (curAnimationIndex++) % (NUM_ANIMATIONS - 1);
      else if (transitionBehavior == TransitionBehavior::Random)
        newAnimationIndex = random(1, NUM_ANIMATIONS);
    } while (!ableToTransition(newAnimationIndex) || newAnimationIndex == curAnimationIndex);
    log_d("Switching to animation %d", newAnimationIndex);
  }
  else
  {
    newAnimationIndex = AnimationType::Name;
  }

  curAnimationIndex = newAnimationIndex;
  curAnimation = animations[newAnimationIndex];
  curAnimation->initialize(Tubes, brightness);
  log_d("Initialized animation %d", newAnimationIndex);
}

void handleRefresh()
{
  if (refreshTick)
  {
    refreshTick = false;

    if (curAnimation->isComplete())
      initializeNewAnimation();

    TickResult result = curAnimation->handleTick(Tubes);

    if (result.CathodeUpdate)
      nixieDisplay(Tubes);
    if (result.BrightnessUpdate)
      nixieBrightness(Tubes);
    // if (refreshTick)
    //   log_w("Refresh took too long");
  }
}

void updateBrightness()
{
  if (millis() - brightnessLastUpdateMillis > brightnessDelayMs)
  {
    log_d("Updating brightness");

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

    isNight = minsToDay < minsToNight;
    brightness = isNight ? nightBrightness : dayBrightness;
    delaySecs = 60 * (isNight ? minsToDay : minsToNight);

    String timeOfDay = isNight ? "night" : "day";
    log_i("Setting brightness to %d (%s), next check in %dm", brightness, timeOfDay, delaySecs/60);

    brightnessDelayMs = delaySecs * 1000;
    brightnessLastUpdateMillis = millis();
  }
}

void setup()
{
  delay(2000);
  Serial.begin(115200);
  log_i("Starting setup");

  nixieSetup();
  wifiSetup();
  otaSetup();
  restSetup();
  mDnsSetup();
  rtcSetup();

  refreshTimer = timerBegin(0, 80, true); // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, false);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  initializeAllAnimations();

  loadSettings();

  // start first animation
  curAnimation = animations[AnimationType::Name];
  curAnimation->initialize(Tubes, brightness);

  log_i("Setup complete");
}

void loop()
{
  handleRefresh();
  updateBrightness();
  checkWifiStatus();
  server.handleClient();
  ArduinoOTA.handle();
}
