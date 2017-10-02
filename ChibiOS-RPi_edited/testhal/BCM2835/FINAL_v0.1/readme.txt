** FINAL V0.1 ** 

Finally something that works!! This version uses the RaspPi to directly poll the N64 controller (pin 18 PWM output, pin 26 input), and the MCP4725 DAC to output audio. Audio samplerate is 11025kHz. Only a few sounds, looping works (START button to control loop, Left-hand-button to stop it). See schematic_v0.1.jpg to see how it’s connected. 

To run, run command: “make” in this directory, and after the build, rename “build/ch.bin” as “build/kernel.img”, and move it to SD card’s root directory.

Notice, I hacked (modified) certain files of the Chibi-OS library:

  ChibiOS-RPi_edited/os/hal/platform/BCM2835/hal_lld.c
  * Changed ARM_TIMER_DIV as so that ARM_TIMER_VAL ticks every 0.5us
  * Changed interrupt loop as so whenever a Pin26 interrupt occurs, record the time
  * wrote methods read_incrementer and read_times to report Pin26 interrupt times
	
  ChibiOS-RPi_edited/os/hal/platform/BCM2835/hal_lld.h
  * Declarations of read_incrementer, read_times from above 

  ChibiOS-RPi_edited/os/hal/platform/BCM2835/pwm_lld.c
  * Changed default divider so that PWM runs at 9.6MHz instead of 600kHz. 
  * (Needed for Raspberry Pi polling PWM)