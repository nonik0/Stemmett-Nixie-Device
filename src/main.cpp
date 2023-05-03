#include "animation.h"
#include "ota.h"
#include "nixieDriver.h"
#include "rtc.h"
#include "tubeConfiguration.h"
#include "tubes.h"

#define NUM_ANIMATIONS  4
Animation* animations[NUM_ANIMATIONS] = {
  new NameAnimation(),
  new NameGlitchAnimation(),
  new FadeAnimation(),
  new SlotMachineAnimation(),
  //new RandomScanAnimation()
};
Animation *curAnimation;

hw_timer_t *refreshTimer;
volatile bool refreshTick = false;

uint8_t randomDigit(TubeType type) {
  return TubeCathodes[type][random(TubeCathodeCount[type])];
}

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
}


void handleRefresh() {
  if (refreshTick) {
    refreshTick = false;

    // if animation is complete, initialize new animation
    if (curAnimation->isComplete()) {
      curAnimation = (curAnimation == animations[0])
        ? animations[random(1, NUM_ANIMATIONS)]
        : animations[0];
      curAnimation->initialize(Tubes);
    }

    TickResult result = curAnimation->handleTick(Tubes);

    if (result.CathodeUpdate) nixieDisplay(Tubes);
    if (result.BrightnessUpdate) nixieBrightness(Tubes);
    if (refreshTick) Serial.println("Refresh took too long");
  }
}

void setup() {
  delay(3000);

  Serial.begin(115200);
  Serial.println("setup()");

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 0; i < NUM_TUBES; i++) {
    pinMode(Tubes[i].AnodePin, OUTPUT);
    digitalWrite(Tubes[i].AnodePin, LOW);
    //pwm.write(Tubes[i].AnodePin, PWM_MIN, PWM_FREQUENCY);
  }

  SPI.begin();

  // draws a lot of current, should do before enabling tubes
  otaSetup();

  rtcSetup();

  refreshTimer = timerBegin(0, 80, true);  // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, true);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  // calculate PWM phase offset for each tube to smooth out power consumption
  for (int i = 0; i < NUM_TUBES; i++) {
    int phaseDeg = (360 / NUM_TUBES) * i;
    Tubes[i].PwmPhase = (phaseDeg / 360.0) * (pow(2,PWM_RESOLUTION));
    pwm.write(Tubes[i].AnodePin, 128, PWM_FREQUENCY, PWM_RESOLUTION, Tubes[i].PwmPhase);
  }

  // start first animation
  curAnimation = animations[0];
  curAnimation->initialize(Tubes);

  Serial.println("Setup complete");
}

void loop() {
  handleRefresh();
  ArduinoOTA.handle();
}


