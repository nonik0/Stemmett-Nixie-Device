# Stemmett Nixie Device

## Description
A Nixie device that uses PL31 socket compatible Nixie tube ([IN-4](https://www.swissnixie.com/tubes/IN4/), [IN-7](https://www.swissnixie.com/tubes/IN7/), [IN-7A](https://www.swissnixie.com/tubes/IN7A/), IN-7B) in different orientations to create makeshift letters and spell things like names! The main board itself has 6 sockets for PL31 breakout boards that are part of this project. These PL31 socket breakout boards have the sockets in different orientations to rotate the symbols in the Nixie tubes. By rotating the symbols we can pseudo-spell things, see images below for example (i.e. "7" rotated 180° to be an "L", or "m" rotated -90° to be a small-sized "E").

The hardware is designed in KiCad and all design files are included in the hardware directory. The hardware consists of the design of the main board PCB and three different designs for the PL31 socket breakout boards.

The firmware is a PlatformIO-based project targetting the [TinyS3](https://esp32s3.com/tinys3.html) and all source files are included in the firmware directory. It makes the Nixies do fancy flashy animatey things while pausing to show the configured name/word between each animation.

## Work Left
- simple REST/HTTP interface to be able to change settings (brightness, animations to include/skip, animation speed)
- simple QoL features: night mode, alarm
- a nice case for both pieces of hardware I've built for two little persons
- more animations are always nice
- is it even worth looking into using an RTOS at this point?

## Images

<img width="1409" alt="circuitboard" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/f3ae704e-cc7b-4b4a-8c89-467c230d003f">

<img width="1062" alt="photoof2devices" src="https://github.com/nonik0/Stemmett-Nixie-Device/assets/17152317/243bab7f-05ea-454b-98be-dd6fbc3197f3">

## Video 

TODO
