#include <Arduino.h>
#include <SPI.h>
#include "tubeConfiguration.h"
#include "tubes.h"

void nixieSetup() {
  log_d("Setting up nixie tubes");
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 0; i < NUM_TUBES; i++) {
    pinMode(Tubes[i].AnodePin, OUTPUT);
    digitalWrite(Tubes[i].AnodePin, LOW);
  }

  SPI.begin();

  // zero out cathode buffer
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  // setup PWM for each tube
  for (int i = 0; i < NUM_TUBES; i++) {
    ledcAttach(Tubes[i].AnodePin, PWM_FREQUENCY, PWM_RESOLUTION);

    // calculate PWM phase offset for each tube to smooth out power consumption
    int phaseDeg = (360 / NUM_TUBES) * i;
    Tubes[i].PwmPhase = (phaseDeg / 360.0) * (pow(2, PWM_RESOLUTION));

    // Set initial brightness
    ledcWrite(Tubes[i].AnodePin, 128);
  }

  log_d("Nixie tubes setup complete");
}

void nixieDisplay(Tube tubes[NUM_TUBES]) {
  // each tube (at most 10 digits/cathodes) is encoded by 4 bits, so need 3
  // total bytes to shift out all tube data to three 8-bit registers
  digitalWrite(LATCH_PIN, LOW);

  for (int i = NUM_TUBES - 1; i >= 1; i -= 2) {
    uint8_t nibble1 = tubes[i].ActiveCathode;
    uint8_t nibble2 = tubes[i - 1].ActiveCathode;
    SPI.transfer((0xF0 & (nibble1 << 4)) | (0x0F & nibble2));
  }

  digitalWrite(LATCH_PIN, HIGH);
}

void nixieBrightness(Tube tubes[NUM_TUBES]) {
  for (int i = 0; i < NUM_TUBES; i++) {
    ledcWrite(tubes[i].AnodePin, tubes[i].Brightness);
  }
}

void nixieOff(Tube tubes[NUM_TUBES]) {
  for (int i = 0; i < NUM_TUBES; i++) {
    ledcWrite(tubes[i].AnodePin, 0);
  }
}
