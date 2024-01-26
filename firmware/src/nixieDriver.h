#include <Arduino.h>
#include <pwmWrite.h>
#include <SPI.h>

Pwm pwm;

void nixieDisplay(Tube tubes[NUM_TUBES]) {
  // each tube (at most 10 digits/cathodes) is encoded by 4 bits, so need 3
  // total bytes to shift out all tube data to three 8-bit registers
  digitalWrite(LATCH_PIN, LOW);

  for (int i = NUM_TUBES-1; i >= 1; i -= 2) { 
    uint8_t nibble1 = tubes[i].ActiveCathode;
    uint8_t nibble2 = tubes[i-1].ActiveCathode;
    SPI.transfer((0xF0 & (nibble1 << 4)) | (0x0F & nibble2));
  }

  digitalWrite(LATCH_PIN, HIGH);
}

void nixieBrightness(Tube tubes[NUM_TUBES]) {
    //Serial.printf("B:%3d|%3d|%3d|%3d|%3d|%3d\n", tubes[5].Brightness, tubes[4].Brightness, tubes[3].Brightness, tubes[2].Brightness, tubes[1].Brightness, tubes[0].Brightness);
    for (int i = 0; i < NUM_TUBES; i++) {  
      pwm.write(tubes[i].AnodePin, tubes[i].Brightness, PWM_FREQUENCY, PWM_RESOLUTION, tubes[i].PwmPhase);
    }
}