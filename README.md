# Stemmett Nixie Device

## Description
A Nixie device that uses front-viewed [PL31 socket](https://tubes-store.com/product_info.php?products_id=101) Nixie tubes [IN-4](https://www.tube-tester.com/sites/nixie/data/in-4/in-4-sh2.htm), [IN-7](https://www.tube-tester.com/sites/nixie/data/IN-7/in-7.htm), [IN-7A](https://www.swissnixie.com/tubes/IN7A/), IN-7B) in different orientations to create makeshift letters and spell things like names! The main board itself has 6 sockets for custom PL31 breakout boards, which have the nixie tube sockets in different orientations to rotate the Nixie tubes. By rotating the tubes we can pseudo-spell things, see images below for example (i.e. "7" rotated 180° to be an "L", or "m" rotated -90° to be a small-sized "E").

The hardware is designed in KiCad and all design files are included in the hardware directory. The hardware consists of the design of the main board PCB and three different versions of the PL31 socket breakout boards. The overall component choices target using simple through-hole components so the PCB is easier to debug, as it is my first PCB design. All cathodes of the nixie tubes are controlled with a single pin with serial, using three 74HC595 shift registers to control the input of the six old school К155ИД1 (K155ID1, SN74141) driver chips for each nixie tube. Each tube anode has a simple MOSFET circuit to be able to PWM each tube individually, allowing each tube to be individually dimmed or turned off. I am using the tiny awesome [NCH8200HV](https://www.tindie.com/products/omnixie/nch8200hv-nixie-high-voltage-power-module/) for the high voltage supply.

The firmware is a PlatformIO-based project targetting the [TinyS3](https://esp32s3.com/tinys3.html) ESP32 dev board. All source files are included in the firmware directory. The firmware will cycle through various different animations, returning to the configured name/word between each. It has a basic HTTP UX with REST interface to update settings--enable/disable animations, changing brightness, etc. It is simple to customize the firmware to any tube configuration by creating a new one in [tubeConfiguration.h](firmware/tubeConfiguration.h).

## Work Left
- a nice case for both pieces of hardware I've built so far for two little persons
- more animations are always nice
- general debugging
- BOM and better docs maybe if interest

## Images

<img width="1409" alt="circuitboard" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/f3ae704e-cc7b-4b4a-8c89-467c230d003f">
<img width="1062" alt="photoof2devices" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/243bab7f-05ea-454b-98be-dd6fbc3197f3">

## Animations

<img width="400" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/ccc5fb62-b015-490d-8923-8d73e164e0a2" />
<img width="400" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/f9963695-3fc3-45b0-a612-ad508b737c26" />
<img width="400" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/8f2c7b56-fa22-4590-b4f3-87f4bfd95e07" />
<img width="400" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/69eb9db4-b7cb-4ffd-b9ef-756fb2e60224" />
<img width="400" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/cf13f3ec-e01d-4733-bef6-d13dc563a7ce" />

TODO: animations in better lighting...
