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
  unsigned int ra;
  unsigned int mem = GPIO_BASE+(BYTESPERBLOCK*((g)/10));
  ra=GET32(mem);
  ra&=~(7<<(((g)%10)*3));

  unsigned int alt_func[6] = {0b100, 0b101, 0b110, 0b111, 0b011, 0b010} ;
  unsigned alt = alt_func[a];
  ra|=(alt<<(((g)%10)*3)); //works!
  //ra|=(0b100<<(((g)%10)*3));
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
  sdStart(&SD1, NULL);
  chprintf((BaseSequentialStream *)&SD1, "I2S test\r\n");


  int g,rep;
  unsigned int ra, mem;
  chprintf((BaseSequentialStream *)&SD1, "Setting GPIO regs to alt0\r\n");
  for (g=18; g<=21; g++)
  {
    INP_GPIO(g);            // tested
    SET_GPIO_ALT(g,0);      // tested
  }


  // printf("Disabling I2S clock\n");
  // *(clk+0x26) = 0x5A000000;
  // *(clk+0x27) = 0x5A000000;

  chprintf((BaseSequentialStream *)&SD1, "Disabling I2S clock\r\n");
  mem = CLOCK_BASE+(BYTESPERBLOCK*0x26);
  PUT32(mem, 0x5A000000);  // sets clock source to nothing!!
  mem = CLOCK_BASE+(BYTESPERBLOCK*0x27);
  PUT32(mem, 0x5A000000);

  chThdSleepMicroseconds(10);

  // printf("Confiure I2S clock\n");
  // *(clk+0x26) = 0x5A000001;
  // *(clk+0x27) = 0x5A000000 | 3<<12 | 1<<9; // divider: 3.125==0b11.001

  chprintf((BaseSequentialStream *)&SD1, "Confiure I2S clock\r\n");
  mem = CLOCK_BASE+(BYTESPERBLOCK*0x26);
  PUT32(mem, 0x5A000001);  // sets clock source to oscillator
  mem = CLOCK_BASE+(BYTESPERBLOCK*0x27);
  ra = 0x5A000000 | 3<<12 | 1<<9;
  // ra = 0x5A000000 | ((3<<12 | 1<<9) << 0); change the ZERO; the larger, the smallef the fq
  // ra = 0x5A0070D7;
  PUT32(mem, ra); // divider: 3.125==0b11.001
  // and ... 19.2MHz / (3.125 * 32) = 192 kHz.*/

  // PUT32(mem, ra); // divider: 3.125==0b11.001
  // and ... 19.2MHz / (3.125 * 32) / 4 = 48 kHz.

  chThdSleepMicroseconds(10);


  // printf("Enabling I2S clock\n");
  // *(clk+0x26) = 0x5A000011;
  chprintf((BaseSequentialStream *)&SD1, "Enabling I2S clock\r\n");
  mem = CLOCK_BASE+(BYTESPERBLOCK*0x26);
  PUT32(mem, 0x5A000011); // enables the clock generator. and oscillator.


  // disable I2S so we can modify the regs
  // printf("Disable I2S\n");
  // *(i2s+0) &= ~(1<<24);
  // usleep(100);
  // *(i2s+0) = 0;
  // usleep(100);

  // disable I2S so we can modify the regs
  chprintf((BaseSequentialStream *)&SD1, "Disable I2S\r\n");
  mem = I2S_BASE;
  ra = GET32(mem);
  ra &= ~(1<<24); // disables sync helper
  PUT32(mem, ra);
  chThdSleepMicroseconds(100);
  mem = I2S_BASE;
  PUT32(mem, 0);  // turns everything off
  chThdSleepMicroseconds(100);

  // printf("Clearing FIFOs\n");
  // *(i2s+0) |= 1<<3 | 1<<4 | 11<5; // clear TX FIFO
  // usleep(10);

  // TYPO WITH 11<5 ? SHOULD BE 11<<5??

  chprintf((BaseSequentialStream *)&SD1, "Clearing FIFOs\r\n");
  mem = I2S_BASE;
  ra = GET32(mem);
  ra |= 1<<3 | 1<<4 | 11<<5; // clear TX, RX FIFOs and something else?
  // 11 = 0x1011 = (1<<0 | 1<<1 | 1<<3)
  // 11<<5 = (1<<5 | 1<<6 | 1<<8)
  // 5:6 Sets the TX FIFO threshold at which point the TXW flag is sets:

  PUT32(mem, ra);
  chThdSleepMicroseconds(10);



  // set register settings
  // --> enable Channel1 with 32bit width
  chprintf((BaseSequentialStream *)&SD1, "Setting TX //channel settings\r\n");
  mem = I2S_BASE+(BYTESPERBLOCK*4); // 0x10
  ra = 1<<30  | 1<<14 //enable channel 1 & 2
        | 8<<16  | 8     //       channel 1 & 2 width
        | 31<<20 | 15<4; //  ch1 pos: 31, ch2 pos = bit 15





  // OLD S1<<31 | 1<<30 | 8<<16;




  //ra = (1<<31 | 1<<30 | 8<<16 | 1<<15 | 1<<14 | 8) ; // and the same for channel 2
  PUT32(mem, ra); // width extention bit; channel 1 enable; ch1 width is 16 + 8 + 8 = 32.
  // --> frame width 31+1 bit
  mem = I2S_BASE+(BYTESPERBLOCK*2);
  ra = 31<<10 | 16;  // bits 19:10 FLEN , 9:0 FSLEN 16



  // OLD 31<<10;
  PUT32(mem, ra);

  // --> disable STBY
  chprintf((BaseSequentialStream *)&SD1, "disabling standby\r\n");
  mem = I2S_BASE;
  ra = GET32(mem);
  ra |= 1<<25;
  PUT32(mem, ra);
  chThdSleepMicroseconds(50);


  // enable I2S
  mem = I2S_BASE;
  ra = GET32(mem);
  ra |= 0x01;
  PUT32(mem, ra);


  // enable transmission
  mem = I2S_BASE;
  ra = GET32(mem);
  ra |= 0x04;
  PUT32(mem, ra);


  // --> ENABLE SYNC bit
  chprintf((BaseSequentialStream *)&SD1, "setting sync bit high\r\n");
  mem = I2S_BASE;
  ra = GET32(mem);
  ra |= 1<<24;
  PUT32(mem, ra);



  ra = GET32(I2S_BASE);
  if (ra & 1<<24) {
   chprintf((BaseSequentialStream *)&SD1, "SYNC bit high, strange.\r\n");
   //PUT32(GPCLR0,1<<16);
   //PUT32(GPSET0,1<<16);
  } else {
   chprintf((BaseSequentialStream *)&SD1, "SYNC bit low, as expected.\r\n");
  }

  chThdSleepMicroseconds(1);
  ra = GET32(I2S_BASE);
  if (ra & 1<<24) {
   chprintf((BaseSequentialStream *)&SD1, "SYNC bit high, as expected.\r\n");
  } else {
   chprintf((BaseSequentialStream *)&SD1, "SYNC bit low, strange.\r\n");
   //PUT32(GPCLR0,1<<16);
   //PUT32(GPSET0,1<<16);
  }


  chprintf((BaseSequentialStream *)&SD1, "Memory dump\r\n");
  for(int i=0; i<9;i++) {
   mem = I2S_BASE + (BYTESPERBLOCK*i);
   ra = GET32(mem);
   chprintf((BaseSequentialStream *)&SD1, "I2S memory address=0x%08x: 0x%08x\r\n", mem, ra);
  }


  OUT_GPIO(16); // LED

  PUT32(GPCLR0,1<<16);
  PUT32(GPSET0,1<<16);



  // fill FIFO in while loop
  int count=0;
  chprintf((BaseSequentialStream *)&SD1, "going into loop\r\n");

  while (1) {

   mem = I2S_BASE + (BYTESPERBLOCK);
   //ra = 0xA0A0A0A0;
   ra = 0xA0A0A0A0;
   PUT32(mem, ra);

  }

  if (1) {

   //chThdSleepMilliseconds(1000);
   PUT32(GPCLR0,1<<16);
   PUT32(GPSET0,1<<16);

   ra = GET32(I2S_BASE);
   while (ra & (1<<19)) {
     // FIFO accepts data
     mem = I2S_BASE + (BYTESPERBLOCK);
     //ra = 0xA0A0A0A0;
     ra = 0xFFFFFFFF;
     PUT32(mem, ra);

     if ((++count % 1000000)==0)
        chprintf((BaseSequentialStream *)&SD1, "Filling FIFO, count=%d\r\n", count);

     ra = GET32(I2S_BASE);
   }

   /*

   mem = I2S_BASE;
   ra = GET32(mem);

     for (int i = 31; i >= 0; i--) {


       if (ra & (1 << i)) {
         chprintf((BaseSequentialStream *)&SD1, "1");
       } else {
         chprintf((BaseSequentialStream *)&SD1, "0");
       }

       if (i % 8 == 0) {
         chprintf((BaseSequentialStream *)&SD1, " ");
       }


     chprintf((BaseSequentialStream *)&SD1, "test");
     chprintf((BaseSequentialStream *)&SD1, "\r\n");
   }
   old_ra = ra;


   if (ra & (1<<19)) {
     // FIFO accepts data
     mem = I2S_BASE+(BYTESPERBLOCK*1);
     ra = 0xA0A0A0A0;
     //ra = counter;
     //ra = 1;
     PUT32(mem, ra);
     /*
     PUT32(GPCLR0,1<<16);
     PUT32(GPSET0,1<<16);

     counter ++;*/

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
