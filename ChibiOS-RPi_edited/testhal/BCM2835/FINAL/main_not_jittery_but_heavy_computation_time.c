/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "dangelo.h"


#define PUT32(a,b) ((*(volatile unsigned int *)a) = (b))
#define GET32(a) (*(volatile unsigned int *) a)
#define BSC0  0x20205000

#define noop



//static MUTEX_DECL(mutex1);
//static CONDVAR_DECL(cond1);



uint32_t mem = 0;
uint32_t ra = 0;
uint32_t on;
uint16_t state = 0;


uint8_t ready = 1;
uint16_t index;
uint32_t counter = 0;


uint32_t controller_state = 0; // is polling done?
uint32_t old_controller_state = 0;

uint8_t sample_done = 0;


#define BUTTON_A      0
#define BUTTON_B      1
#define BUTTON_Z      2
#define BUTTON_START  3
#define DPAD_UP       4
#define DPAD_DOWN     5
#define DPAD_LEFT     6
#define DPAD_RIGHT    7
#define BUTTON_LEFT   10
#define BUTTON_RIGHT  11
#define CPAD_UP       12
#define CPAD_DOWN     13
#define CPAD_LEFT     14
#define CPAD_RIGHT    15
#define XAXIS_START   16
#define YAXIS_START   24



#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028


#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define I2S_BASE                (BCM2708_PERI_BASE + 0x203000) /* GPIO controller */
#define CLOCK_BASE               (BCM2708_PERI_BASE + 0x101000) /* Clocks */

#define noop

#define BYTESPERBLOCK 4


void INP_GPIO(unsigned int g) {
  unsigned int ra;
  unsigned int mem = GPIO_BASE+(BYTESPERBLOCK*((g)/10));
  ra=GET32(mem);
  ra&=~(7<<(((g)%10)*3));
  PUT32(mem,ra);
}

static const uint8_t mcp4725_address = 0x60;

static void play_sample() { //float millivolts) {

  // HACK
  /*
  if (counter % 2 == 0) {
    level = 0;
    //palClearPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    //play_sample(0); // 0b111111111111 / 2);
  } else {
    level = 0b111111111111;
    //palSetPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    //play_sample(0b111111111111);
  }*/

  uint16_t level = data[counter];;
  uint8_t request[2];
  request[0] = (level >> 8) & 0xff;
  request[1] = level & 0xff;

  i2cAcquireBus(&I2C0);


  msg_t status = i2cMasterTransmitTimeout(
    &I2C0, mcp4725_address, request, 2,
    NULL, 0, TIME_INFINITE); //MS2ST(100));
  i2cReleaseBus(&I2C0);

  //if (status != RDY_OK) { }
}


static void interrupt_loop(GPTDriver *gptp) {
  UNUSED(gptp);
  chSysLockFromIsr();

  // queue audio to be played!
  ready = 1;

  chSysUnlockFromIsr();
}


void generate_sample(uint32_t sample) {
  mem = BSC0 + 0x10; // address of FIFO
  PUT32(mem, sample);
  return;
}

void write_sample(uint32_t sample) {
  mem = BSC0 + 0x10; // address of FIFO
  PUT32(mem, sample);
  return;
}

/*
 * Application entry point.
 */
int main(void) {
  halInit();
  chSysInit();

  /*
   * Serial port initialization.
   */
  sdStart(&SD1, NULL);
  chprintf((BaseSequentialStream *)&SD1, "BCM2835 GPT Demonstration\r\n");



  /*
   * Serial General Purpose Timer (GPT) #1 initialization.
   */
  GPTConfig gptConfig1;
  gptConfig1.callback = interrupt_loop;
  gptStart(&GPTD2, &gptConfig1);

  /*
   * I2C initialization.
   */
  I2CConfig i2cConfig;
  //i2cConfig.ic_speed = 1000000; // 1 MHz no?
  i2cConfig.ic_speed =    500000; // .1 MHz no?
  i2cStart(&I2C0, &i2cConfig);




  /*
   *  initialize LED (pin 8)
   */

     palSetPadMode(ONBOARD_LED_PORT, ONBOARD_LED_PAD, PAL_MODE_OUTPUT);

  // TRIES
  //gptStartContinuous(&GPTD2, 50); // This is more or less 20kHz. can be more precise.
  // 10 kHz audio.
  gptStartContinuous(&GPTD2, 91); // This is more or less 44.1kHz / 4. can be more precise.






  while (1) {

    if (ready == 0)
      continue;
    // plays sample
    play_sample();

    counter ++;
    if (counter == 4*55125)
      counter = 0;

    palClearPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    palSetPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);

    // sets ready blocker to zero
    ready = 0;
  }



  while (1)
    noop;
  return 0;
}


/*

  // write buffer (1/2)
  mem = BSC0 + 0x04; // STATUS
  ra = GET32(mem);
  while (!(ra && (1 << 4))) { //if FIFO can accept data
    ra = GET32(mem);
  }
  mem = BSC0 + 0x10; // put into fifo
  ra = (state >> 8) & 0xff;
  PUT32(mem, ra);


  // write buffer (2/2)
  ra = GET32(mem);
  while (!(ra && (1 << 4))) { //if FIFO can accept data
    ra = GET32(mem);
  }
  mem = BSC0 + 0x10; // put into fifo
  ra = state & 0xff;
  PUT32(mem, state);




*/
