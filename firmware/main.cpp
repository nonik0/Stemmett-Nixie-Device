#include "Animation/animation.h"
#include "nixieDriver.h"
#include "rest.h"
#include "rtc.h"
#include "services.h"
#include "tubeConfiguration.h"
#include "tubes.h"

Animation* animations[NUM_ANIMATIONS];
Animation* curAnimation;
int curAnimationIndex = 0;

hw_timer_t* refreshTimer;
volatile bool refreshTick = false;

int brightness = 150;
long brightnessDelayMs = -1;

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
}

bool ableToTransition() {
  // check if any animations are enabled other than the name animation
  for (int i = 1; i < NUM_ANIMATIONS; i++)
    if (animationsEnabled[i]) return true;
  return false;
}

void initializeAllAnimations() {
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
  for (int i = 0; i < NUM_ANIMATIONS; i++) {
    animationsEnabled[i] = true;
  }
}

void initializeNewAnimation() {
  log_d("Initializing new animation");

  int newAnimationIndex;
  if (curAnimationIndex == AnimationType::Name && ableToTransition()) {
    do {
      if (transitionBehavior == TransitionBehavior::Sequential)
        newAnimationIndex = 1 + (curAnimationIndex++) % (NUM_ANIMATIONS - 1);
      else if (transitionBehavior == TransitionBehavior::Random)
        newAnimationIndex = random(1, NUM_ANIMATIONS);
    } while (!animationsEnabled[newAnimationIndex] || newAnimationIndex == curAnimationIndex);
    log_d("Switching to animation %d", newAnimationIndex);
  } else {
    newAnimationIndex = AnimationType::Name;
  }

  curAnimationIndex = newAnimationIndex;
  curAnimation = animations[newAnimationIndex];
  curAnimation->initialize(Tubes, brightness);
  log_d("Initialized animation %d", newAnimationIndex);
}

void handleRefresh() {
  if (refreshTick) {
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

void updateBrightness() {
  if (brightnessDelayMs < 0) {
    int delaySecs, hour, minute, second;

    rtcGetTime(hour, minute, second);

    int curMins = hour * 60 + minute;
    int minsToDay = (dayTransitionTime.tm_hour * 60 + dayTransitionTime.tm_min - curMins + 1440) % 1440;
    int minsToNight = (nightTransitionTime.tm_hour * 60 + nightTransitionTime.tm_min - curMins + 1440) % 1440;

    isNight = minsToDay < minsToNight;
    brightness = isNight ? nightBrightness : dayBrightness;
    delaySecs = isNight ? minsToDay : minsToNight;

    String timeOfDay = isNight ? "night" : "day";
    log_i("Setting brightness to %d (%s)", brightness, timeOfDay);

    brightnessDelayMs = delaySecs * 1000;
  }
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  log_i("Starting setup");

  nixieSetup();
  wifiSetup();
  otaSetup();
  restSetup();
  mDnsSetup();
  rtcSetup();

  refreshTimer = timerBegin(0, 80, true);  // 80Mhz / 80 = 1Mhz
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

void loop() {
  handleRefresh();
  updateBrightness();
  checkWifiStatus();
  server.handleClient();
  ArduinoOTA.handle();
}
