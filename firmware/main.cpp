#include "Animation/animation.h"
#include "nixieDriver.h"
#include "ota.h"
#include "rest.h"
#include "rtc.h"
#include "tubeConfiguration.h"
#include "tubes.h"

TransitionBehavior transitionBehavior = TransitionBehavior::Sequential;
bool animationsEnabled[NUM_ANIMATIONS];
Animation* animations[NUM_ANIMATIONS];
Animation* curAnimation;
int i = 0;
int curAnimationIndex = 0;
int newAnimationIndex = 0;
int brightness = 150;  // MAX_BRIGHTNESS;

hw_timer_t* refreshTimer;
volatile bool refreshTick = false;

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
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
  log_i("Initializing new animation");

  if (curAnimationIndex == AnimationType::Name) {
    do {
      if (transitionBehavior == TransitionBehavior::Sequential)
        newAnimationIndex = 1 + (i++) % (NUM_ANIMATIONS - 1);
      else if (transitionBehavior == TransitionBehavior::Random)
        newAnimationIndex = random(1, NUM_ANIMATIONS);
    } while (!animationsEnabled[newAnimationIndex] ||
             newAnimationIndex == curAnimationIndex);
    log_i("Switching to animation %d", newAnimationIndex);
  } else {
    newAnimationIndex = AnimationType::Name;
  }

  curAnimationIndex = newAnimationIndex;
  curAnimation = animations[newAnimationIndex];
  curAnimation->initialize(Tubes, brightness);
  log_i("Initialized animation %d", newAnimationIndex);
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

void setup() {
  delay(2000);
  Serial.begin(115200);
  log_w("setup()");

  nixieSetup();
  wifiSetup();
  otaSetup();
  restSetup();
  rtcSetup();

  refreshTimer = timerBegin(0, 80, true);  // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, true);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  initializeAllAnimations();

  // start first animation
  curAnimation = animations[AnimationType::Name];
  curAnimation->initialize(Tubes, brightness);

  log_w("Setup complete");
}

void loop() {
  handleRefresh();
  checkWifiStatus();
  server.handleClient();
  ArduinoOTA.handle();
}
