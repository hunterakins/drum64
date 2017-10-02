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

// directly copied from code

#define PUT32(a,b) ((*(volatile unsigned int *)a) = (b))
#define GET32(a) (*(volatile unsigned int *) a)

extern void dummy ( unsigned int );

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

void OUT_GPIO(unsigned int g) {
  unsigned int ra;
  unsigned int mem = GPIO_BASE+(BYTESPERBLOCK*((g)/10));
  ra=GET32(mem);
  ra|=(1<<(((g)%10)*3));
  PUT32(mem,ra);
}

void SET_GPIO_ALT(unsigned int g, unsigned int a) {
  unsigned int alt_func[6] = {0b100, 0b101, 0b110, 0b111, 0b011, 0b010} ;
  unsigned int ra;
  unsigned int mem = GPIO_BASE+(BYTESPERBLOCK*(g)/10);
  ra=GET32(mem);
  ra&=~(7<<(((g)%10)*3));
  //ra|=(4<<(((g)%10)*3));
  ra|=(alt_func[a]<<(((g)%10)*3));
  //ra|= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3));
  PUT32(mem,ra);
}


/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  /*
  // this block of code confirms that my set_gpio_alt function works.
  PWMConfig pwmConfig;
  pwmConfig.period = 200;

  pwmStart(&PWMD1, &pwmConfig);
  pwmEnableChannel(&PWMD1, 0, 1);
  SET_GPIO_ALT(12,0);*/
  /*
   * Serial port initialization.
   */
  //sdStart(&SD1, NULL);
  //chprintf((BaseSequentialStream *)&SD1, "BCM2835 GPT Demonstration\r\n");


  int g,rep;
  unsigned int ra, mem;

  //printf("Setting GPIO regs to alt0\n");
  for (g=18; g<=21; g++)
  {
    INP_GPIO(g);
    SET_GPIO_ALT(g,0);
  }

   // //printf("Disabling I2S clock\n");
   mem = CLOCK_BASE+(BYTESPERBLOCK*0x26);
   PUT32(mem, 0x5A000000);  // sets clock source to nothing!!
   mem = CLOCK_BASE+(BYTESPERBLOCK*0x27);
   PUT32(mem, 0x5A000000);

   chThdSleepMicroseconds(10);

   // //printf("Confiure I2S clock\n");
   mem = CLOCK_BASE+(BYTESPERBLOCK*0x26);
   PUT32(mem, 0x5A000001);  // sets clock source to oscillator
   mem = CLOCK_BASE+(BYTESPERBLOCK*0x27);
   /*ra = 0x5A000000 | 3<<12 | 1<<9;
   PUT32(mem, ra); // divider: 3.125==0b11.001
   // and ... 19.2MHz / (3.125 * 32) = 192 kHz.*/

   ra = 0x5A000000 | ((3<<12 | 1<<9) << 2);
   PUT32(mem, ra); // divider: 3.125==0b11.001
   // and ... 19.2MHz / (3.125 * 32) / 4 = 48 kHz.


   chThdSleepMicroseconds(10);
   // //printf("Enabling I2S clock\n");
   mem = CLOCK_BASE+(BYTESPERBLOCK*0x26);
   PUT32(mem, 0x5A000011); // enables the clock generator. and oscillator.



   /////


   // disable I2S so we can modify the regs
   //printf("Disable I2S\n");
   mem = I2S_BASE;
   ra = GET32(mem);
   ra &= ~(1<<24); // disables sync helper
   PUT32(mem, ra);
   chThdSleepMicroseconds(100);
   PUT32(mem, 0);  // turns everything off
   chThdSleepMicroseconds(100);

   //printf("Clearing FIFOs\n");
   ra = GET32(mem);
   ra |= (1<<3 | 1<<4 | 11<5); // clear TX, RX FIFOs and something else?
   PUT32(mem, ra);
   chThdSleepMicroseconds(10);

   // set register settings
   // --> enable Channel1 with 32bit width
   //printf("Setting TX //channel settings\n");
   mem = I2S_BASE+(BYTESPERBLOCK*4); // 0x10
   PUT32(mem, 1<<31 | 1<<30 | 8<<16); // width extention bit; channel 1 enable; ch1 width is 16 + 8 + 8 = 32.
   // --> frame width 31+1 bit
   mem = I2S_BASE+(BYTESPERBLOCK*2);
   PUT32(mem, 31<<10);

   // --> disable STBY
   //printf("disabling standby\n");
   mem = I2S_BASE;
   ra = GET32(mem);
   ra |= 1<<25;
   PUT32(mem, ra);
   chThdSleepMicroseconds(50);



   // enable I2S
   ra = GET32(mem);
   ra |= 0x01;
   PUT32(mem, ra);

   // enable transmission

   ra = GET32(mem);
   ra |= 0x04;
   PUT32(mem, ra);

   // --> ENABLE SYNC bit
   //printf("setting sync bit high\n");
   ra = GET32(mem);
   ra |= 1<<24;
   PUT32(mem, ra);


   ra = GET32(I2S_BASE);
   if (ra & 1<<24) {
     //printf("SYNC bit high, strange.\n");
     //PUT32(GPCLR0,1<<16);
     //PUT32(GPSET0,1<<16);
   } else {
     //printf("SYNC bit low, as expected.\n");
   }

   //chThdSleepMicroseconds(1);

   if (ra & 1<<24) {
     //printf("SYNC bit high, as expected.\n");
   } else {
     //printf("SYNC bit low, strange.\n");
     //PUT32(GPCLR0,1<<16);
     //PUT32(GPSET0,1<<16);
   }

   OUT_GPIO(16);


  int counter = 0;
 while (1) {

   unsigned int mem = I2S_BASE;
   ra = GET32(mem);
   while (ra & (1<<19)) {
     // FIFO accepts data
     mem = I2S_BASE+(BYTESPERBLOCK*1);
     //ra = 0xA0A0A0A0;
     ra = counter;
     //ra = 1;
     PUT32(mem, ra);
     PUT32(GPCLR0,1<<16);
     PUT32(GPSET0,1<<16);

     counter ++;


   }

 }


/*  unsigned int ra;
  unsigned int mem = GPIO_BASE+(BYTESPERBLOCK*((16)/10));
  ra=GET32(mem);
  ra&=~(7<<18);
  ra|=1<<18;
  PUT32(GPFSEL1,ra);

  while(1)
  {
      PUT32(GPSET0,1<<16);

      //palSetPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
      //for(ra=0;ra<0x100000;ra++) noop;
      PUT32(GPCLR0,1<<16);
      //palClearPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
      //for(ra=0;ra<0x100000;ra++) noop;
  }*/
  return(0);

}
