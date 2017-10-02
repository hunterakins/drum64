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

// for audio debugging purposes
//#include "dangelo.h"


#include "0_kick.h"
#include "1_snare.h"
#include "2_tomhi.h"
#include "3_tomlo.h"
#include "4_hihatc.h"
#include "5_hihato.h"
#include "6_ride.h"

#include "current_state.h"

uint32_t sample_sizes[8] = {1258, 2617, 16372, 17109, 1619, 3926, 35325, 0};

#define MAX_SAMPLE_LENGTH 35325

volatile uint16_t samples[8][MAX_SAMPLE_LENGTH];


#define PUT32(a,b) ((*(volatile unsigned int *)a) = (b))
#define GET32(a) (*(volatile unsigned int *) a)
#define BSC0  0x20205000
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028
#define PWMCTL     0x2020C000


#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define I2S_BASE                (BCM2708_PERI_BASE + 0x203000) /* GPIO controller */
#define CLOCK_BASE               (BCM2708_PERI_BASE + 0x101000) /* Clocks */

#define noop

#define BYTESPERBLOCK 4



#define SAMPLERATE 11025  // in Hz
#define POLL_PERIOD 8 // number of samples per poll
#define POLL_READ   3 // number of samples in, to send read signal

//static MUTEX_DECL(mutex1);
//static CONDVAR_DECL(cond1);



volatile uint32_t mem = 0;
volatile uint32_t ra = 0;
volatile uint32_t on;
volatile uint16_t state = 0;

volatile uint16_t old = 0; // old counter time

uint32_t counter = 10;


#define AUDIO_BUFFER_SIZE 128

volatile uint16_t audio_buffer[AUDIO_BUFFER_SIZE];
volatile uint16_t audio_buffer_counter;


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



// load samples!!
void load_samples(void) {
  volatile uint16_t sample;
  for (unsigned int i = 0; i < 8; i++) {
    for (unsigned int j = 0; j < MAX_SAMPLE_LENGTH; j++) {
      if (j >= sample_sizes[i]) {
        samples[i][j] = 0x7FF;
      } else {
        if (i == 0) {
          sample = data_0_kick[j];
        } else if (i == 1) {
          sample = data_1_snare[j];
        } else if (i == 2) {
          sample = data_2_tomhi[j];
        } else if (i == 3) {
          sample = data_3_tomlo[j];
        } else if (i == 4) {
          sample = data_4_hihatc[j] ; //0x7FF + ((data_4_hihatc[j] - 0x7FF) >> 2); //HACKS HIHAT IS TOO LOUD
        } else if (i == 5) {
          sample = data_5_hihato[j];
        } else if (i == 6) {
          sample = data_6_ride[j];
        }

        samples[i][j] = sample;
      }

    }
  }
}


void INP_GPIO(unsigned int g) {
  unsigned int ra;
  unsigned int mem = GPIO_BASE+(BYTESPERBLOCK*((g)/10));
  ra=GET32(mem);
  ra&=~(7<<(((g)%10)*3));
  PUT32(mem,ra);
}


/*
 * Uses Chibi-OS's drivers to initialize system. A real hack.
 */
static const uint8_t mcp4725_address = 0x60;

void init_system_I2C(void) {
  //uint16_t level = data[counter];
  uint16_t level = 0xffff;
  uint8_t request[2];
  request[0] = (level >> 8) & 0xff;
  request[1] = level & 0xff;

  i2cAcquireBus(&I2C0);


  //msg_t status =
  i2cMasterTransmitTimeout(
    &I2C0, mcp4725_address, request, 2,
    NULL, 0, TIME_INFINITE); //MS2ST(100));
  i2cReleaseBus(&I2C0);
}


void write_sample(GPTDriver *gptp) { // takes about 3us of computation
  UNUSED(gptp);
  chSysLockFromIsr();

  // write buffer (2/2)
  mem = BSC0 + 0x10; // put into fifo
  ra = (audio_buffer[audio_buffer_counter] >> 4) & 0xff;
  // changed from ra = audio_buffer[audio_buffer_counter] & 0xff;
  // and ra = (audio_buffer[(audio_buffer_counter+1) % AUDIO_BUFFER_SIZE] >> 8) & 0xff;
  // so that overflow doesn't happen!
  PUT32(mem, ra);

  // set bits to WRITE
  mem = BSC0; // STATUS
  ra = GET32(mem);
  ra |= (BSC_INTT | BSC_INTD | START_WRITE);;
  PUT32(mem, ra);

  // write buffer (1/2)
  mem = BSC0 + 0x10; // put into fifo
  ra = (audio_buffer[(audio_buffer_counter+1) % AUDIO_BUFFER_SIZE] >> 12) & 0xff;
  PUT32(mem, ra);

  // increment audio buffer
  audio_buffer_counter++;
  if (audio_buffer_counter == AUDIO_BUFFER_SIZE)
    audio_buffer_counter = 0;

  chSysUnlockFromIsr();
}

void update_state(CurrentState *cstate, uint32_t controller_state, uint32_t old_controller_state) {
  //chprintf((BaseSequentialStream *)&SD1, "CONTROLLER STATE %d\r\n", (controller_state));
  cstate->amplitude_X = (controller_state >> XAXIS_START) & 0xff;
  cstate->amplitude_Y = (controller_state >> YAXIS_START) & 0xff;
  cstate->fresh = 1;

  uint8_t drum_button_onset = 0;
  uint8_t loop_onset = 0;
  uint8_t mute_press = 0;
  uint8_t loop_kill_press = 0;

  uint16_t onsets = (controller_state & ~(old_controller_state));
  uint16_t offsets = (~(controller_state) & old_controller_state);
  uint16_t pressed = (controller_state & old_controller_state);

  if (onsets != 0) {
    //chprintf((BaseSequentialStream *)&SD1, "CONTROLLER STATE %d\r\n", (controller_state));
    //chprintf((BaseSequentialStream *)&SD1, "AMPLITUDEX %d\r\n", (controller_state >> XAXIS_START) & 0xff);
    //chprintf((BaseSequentialStream *)&SD1, "ONSETS %u\r\n", onsets);
  }

  for (int i = 0; i < 16; i++) {
    if (onsets & (1 << i)) {
      //chprintf((BaseSequentialStream *)&SD1, "ONSET %d\r\n", i);
      // loop started
      if (i == BUTTON_Z)
        loop_onset = 1;

      // drums should be played.
      else if (i == BUTTON_A)
        drum_button_onset |= (1 << 0);
      else if (i == BUTTON_B)
        drum_button_onset |= (1 << 1);
      else if (i == CPAD_DOWN)
        drum_button_onset |= (1 << 2);
      else if (i == CPAD_LEFT)
        drum_button_onset |= (1 << 3);
      else if (i == CPAD_RIGHT)
        drum_button_onset |= (1 << 4);
      else if (i == CPAD_UP)
        drum_button_onset |= (1 << 5);
      else if (i == BUTTON_RIGHT)
        drum_button_onset |= (1 << 6);
      else if (i == BUTTON_LEFT) // if left button: KILL LOOP
        loop_kill_press = 1;
    }

    // CONTINUAL PRESS
    else if (pressed & (1 << i)) {
      //chprintf((BaseSequentialStream *)&SD1, "CONTINUAL %d\r\n", i);
      // if start button: MUTE DRUMS
      if (i == BUTTON_START)
        mute_press = 1;

      else if (i == BUTTON_LEFT)
        loop_kill_press = 1;
    }

    // OFFSET
    else if (offsets & (1 << i)) {
      // chprintf((BaseSequentialStream *)&SD1, "OFFSET %d\r\n", i);
      // nothing for now ...!
    }
  }

  cstate->drum_button_onset = drum_button_onset;
  cstate->loop_onset = loop_onset;
  cstate->mute_press = mute_press;
  cstate->loop_kill_press = loop_kill_press;

}

/*
 * Figures out the loop state
 * updates only on fresh!!
 */
void update_loop_state(LoopState *lstate, CurrentState *cstate) {

   uint8_t drum_button_onset = cstate->drum_button_onset;
   uint8_t loop_onset        = cstate->loop_onset;
   uint8_t loop_kill_press   = cstate->loop_kill_press;
   uint8_t amplitude_X       = cstate->amplitude_X;
   uint8_t amplitude_Y       = cstate->amplitude_Y;

   if (loop_onset) {
     //chprintf((BaseSequentialStream *)&SD1, "LOOP STARTED\r\n");
     // if loop hasn't even started yet!
     if (lstate->loop_started == 0) {
       if (lstate->loop_length == 0) {
         lstate->loop_started = 1;
         lstate->loop_recording = 1;
       } else { // paused and now resumed
         lstate->loop_started = 1;
       }
     }
     // if loop was pressed just a single time
     else if (lstate->loop_length == 0) {
       lstate->loop_length = lstate->loop_counter+1;
       //lstate->loop_counter = 0; // reset loop_counter to zero!
       lstate->loop_recording = 0; // no longer recording
     } else {
       // toggling between recording and/or not recording
       if (lstate->loop_recording) {
         lstate->loop_recording = 0;
       } else {
         lstate->loop_recording = 1;
       }
     }
   }

   if (loop_kill_press) { // reset everything to 0
     //chprintf((BaseSequentialStream *)&SD1, "KILLED\r\n");
     lstate->loop_started    = 0;
     lstate->loop_recording  = 0;
     lstate->loop_length     = 0;
     lstate->loop_counter    = 0;
   }


   // if loop has started ..! how to update
   if (lstate->loop_started) {
     lstate->loop_counter += 1;
     if (lstate->loop_counter == lstate->loop_length) {
       lstate->loop_counter = 0;
     }

     if (lstate->loop_length == 0) {
       // erase things for good measure!
       for (int i = 0; i < 8; i++) {
         (lstate->loop_amplitude_X)[i][lstate->loop_counter] = 0;
         (lstate->loop_amplitude_Y)[i][lstate->loop_counter] = 0;
       }
     }

     if (lstate->loop_recording) { // if recording!!
       for (int i = 0; i < 8; i++) {
         if (drum_button_onset & (1 << i)) { // if onset of button i!
           //chprintf((BaseSequentialStream *)&SD1, "ONSET %d\r\n", (lstate->loop_counter));
           //chprintf((BaseSequentialStream *)&SD1, "AMPLITUDE %d\r\n", (amplitude_X));
           (lstate->loop_amplitude_X)[i][lstate->loop_counter] = amplitude_X;
           (lstate->loop_amplitude_Y)[i][lstate->loop_counter] = amplitude_Y;
         }
       }
     }
   }



 }

/*
 * INCREMENTS THE DRUM COUNTERS!!
 */
void update_drum_counters(DrumCounters *dcounters, CurrentState *cstate, LoopState *lstate) {

  if (cstate->fresh != 0) { // FRESH!
    // if button ONSET or LOOP ONSET
    for (int i = 0; i < 8; i++) {
      // button ONSET TRUMPS!
      if (cstate->drum_button_onset & (1 << i)) {
        dcounters->drum_playing[i] = 1;
        dcounters->drum_counter[i] = 0;
        dcounters->drum_amplitude_X[i] = cstate->amplitude_X;
        dcounters->drum_amplitude_Y[i] = cstate->amplitude_Y;
      }
      // loop onset!
      else if ((lstate->loop_amplitude_X)[i][lstate->loop_counter] != 0) {
        if (cstate->mute_press == 0) { // is NOT MUTED
          dcounters->drum_playing[i] = 1;
          dcounters->drum_counter[i] = 0;
          dcounters->drum_amplitude_X[i] = lstate->loop_amplitude_X[i][lstate->loop_counter];
          dcounters->drum_amplitude_Y[i] = lstate->loop_amplitude_Y[i][lstate->loop_counter];
        }
      }
    }
  }

  for (int i = 0; i < 8; i++) {

    // increment counter if drum is playing
    if (dcounters->drum_playing[i]) {
      (dcounters->drum_counter)[i] = (dcounters->drum_counter)[i] + 1 ;
      //chprintf((BaseSequentialStream *)&SD1, "Drum Counter %d\r\n", (dcounters->drum_counter)[i]++);

      // if sample reaches end, stop playing it!!

      if ((dcounters->drum_counter)[i] == MAX_SAMPLE_LENGTH) {
        dcounters->drum_playing[i] = 0;
        dcounters->drum_counter[i] = 0;
      }
    }
  }
}

/*
 * GENERATES AUDIO SAMPLE FROM DRUM COUNTERS!!!
 */
uint16_t generate_sample(DrumCounters *dcounters) {

  uint16_t instant_sample = 0x7FFF; // smack dab in the middle between 0 and 0xffff

  for (int i = 0; i < 8; i++) {

    if (dcounters->drum_playing[i]) {
      //if (dcounters->drum_counter[i] == 1)
      //  chprintf((BaseSequentialStream *)&SD1, "DRUM ONSET %d\r\n", i);

      if (i == 4) {
        instant_sample += (samples[i][dcounters->drum_counter[i]] - 0x7FF) >> 2;
      }
      else {
        instant_sample += samples[i][dcounters->drum_counter[i]];
        instant_sample -= 0x7FF;
      }

    }
  }

  //sample = (sample >> 4);
  return instant_sample;
}


/*
 * Analyze Times. Analyzes the N64 interrupt counter times
 * returns button state
*/
uint32_t analyze_times ( uint32_t *times , uint32_t increments_between)
{
  uint32_t state = 0;
  uint8_t j_counter = 32;
  uint8_t high = 0;
  uint8_t current_bit;


  // first bit :
  int i = 0;
  current_bit = times[increments_between - (i + 2)] - times[increments_between - (i + 1)];
  if (current_bit < 6) { //(current_bit < 7) // low gap
    //chprintf((BaseSequentialStream *)&SD1, "0");
  } else if (current_bit > 8) {
    //chprintf((BaseSequentialStream *)&SD1, "2");
    high = 1;
  } else {
    //chprintf((BaseSequentialStream *)&SD1, "1");
  }

  if (high)
    state |= (1 << 31);



  // walk backwards
  // increments_between - 1, increments_between - 2, increments_between - 33 ...
  for (int i = 1; i < 33; i++) {
    current_bit = times[increments_between - (i + 2)] - times[increments_between - (i + 1)];
    if (current_bit < 6) { //(current_bit < 7)
      //chprintf((BaseSequentialStream *)&SD1, "0");
      if (high) {
        high = 0;
      } else {
        // broken
        return (~(0));
        break;
      }
    }
    else if (current_bit > 8) {
      //chprintf((BaseSequentialStream *)&SD1, "2");
      if (~(high)) {
        high = 1;
      } else { // ERROR
        return (~(0));
        break;
      }
    }
    else {
      //chprintf((BaseSequentialStream *)&SD1, "1");
    }

    if (high)
      state |= 1 << (32 - (i + 1));
  }
  //chprintf((BaseSequentialStream *)&SD1, "\r\n");


  return state;


  /*
  for (int i = (increments_between - 32 - 1); i < increments_between; i++) {
    if (((times[i-1] - times[i]) < 7))
      chprintf((BaseSequentialStream *)&SD1, "0");
    else if (((times[i-1] - times[i]) > 8))
      chprintf((BaseSequentialStream *)&SD1, "2");
    else
      chprintf((BaseSequentialStream *)&SD1, "1");
  }

  chprintf((BaseSequentialStream *)&SD1, "\r\n");
  */

  return;
}


/*
 * SPI send poll signal!
 *
 *
 */
void send_poll_signal(void) {
  // write to PWM FIFO
  //change duty cycle
  for (int i = 0; i < 7; i++) { // 3 low 1 high
    ra = 25;
    PUT32(mem, ra);
  }
  for (int i = 0; i < 2; i++) { // 1 low 3 high
    ra = 8;
    PUT32(mem, ra);
  }
  PUT32(mem, 0);
  return;
}

/*
 * READ poll signal!
*/
uint32_t read_poll_signal(void) {

  // do analysis here
  uint32_t temp_state;
  uint32_t ra = read_incrementer();
  uint32_t increments_between = (ra - old);
  //chprintf((BaseSequentialStream *)&SD1, "Number of pulse increments: %d\r\n", increments_between);//increments_between);
  old = ra;

  uint32_t times[64];
  read_times(times);




  temp_state = analyze_times(times, increments_between);
  for (int i = 0; i < 32; i++) {
    if (temp_state & (1 << i)) {
      chprintf((BaseSequentialStream *)&SD1, "1");
    } else {
      chprintf((BaseSequentialStream *)&SD1, "0");
    }
  }

  chprintf((BaseSequentialStream *)&SD1, "\r\n");

  return temp_state;

}


/*
 * READ poll signal! (debug generates drum signal)
*/
uint32_t read_poll_signal_debug(void) {

  uint32_t quarter_note_cycles = 7360;
  uint32_t eigth_note_cycles = quarter_note_cycles / 2;
  uint32_t measure_note_cycles = quarter_note_cycles * 4;
  uint32_t poll_signal = 0x7f7f0000; // mid amplitudes, nothing else
  // debug: return expected button presses
  // bass drums 90 beats beats per minute
  // which is ... approximately every 7360 samples

  /* // rock beat!
  if (counter % quarter_note_cycles == 0) {
    poll_signal |= (1 << BUTTON_A); // just a bass drum
  }

  if (counter % (quarter_note_cycles * 2) == 0) {
    poll_signal |= (1 << BUTTON_B); // just a bass drum
  }

  if (counter % (eigth_note_cycles) == 0) {
    poll_signal |= (1 << CPAD_RIGHT); // just a bass drum
  }*/

  // first measure just hihat
  if (counter < measure_note_cycles) {
    if (counter % (eigth_note_cycles) == 0) {
      poll_signal |= (1 << CPAD_RIGHT); // hihatc
    }
  }
  // second measure, hihat + start loop (recording)
  else if (counter < 2*measure_note_cycles) {
    if (counter % (eigth_note_cycles) == 0) {
      poll_signal |= (1 << CPAD_RIGHT); // hihatc
    }
    if (counter % (measure_note_cycles) == 0) {
      poll_signal |= (1 << BUTTON_Z); // start loop of recording!
    }
  }
  // third measure. no hihat. close loop recording. make sure hihat is looped back
  else if (counter < 3*measure_note_cycles) {
    if (counter % (measure_note_cycles) == 0) {
      poll_signal |= (1 << BUTTON_Z); // close loop of recording!
    }
  }
  // fourth measure. bass drum. stop recording
  else if (counter < 4*measure_note_cycles) {
    if (counter % quarter_note_cycles == 0) {
      poll_signal |= (1 << BUTTON_A); // just a bass drum
    }
  }
  // fifth measure. recording still stopped. make sure it's just hihat that's playing
  else if (counter < 5*measure_note_cycles) {
    noop;
  }
  // sixth measure. bass drum + recording
  else if (counter < 6*measure_note_cycles) {
    if (counter % quarter_note_cycles == 0) {
      poll_signal |= (1 << BUTTON_A); // just a bass drum
    }
    if (counter % (measure_note_cycles) == 0) {
      poll_signal |= (1 << BUTTON_Z); // start loop of recording!
    }
  }
  // seventh measure. snare + recording
  else if (counter < 7*measure_note_cycles) {
    if (counter % (2*quarter_note_cycles) == quarter_note_cycles) {
      poll_signal |= (1 << BUTTON_B); // snare drum
    }
  }
  // eigth measure. stop recording
  else if (counter < 8*measure_note_cycles) {
    if (counter % (measure_note_cycles) == 0) {
      poll_signal |= (1 << BUTTON_Z); // stop recording loop of recording!
    }
  }
  // ninth measure. mute the measure. hihat, crash, toms
  else if (counter < 9*measure_note_cycles) {
    if (counter % (measure_note_cycles) < (measure_note_cycles / 2)) {
      poll_signal |= (1 << BUTTON_START); // MUTE RECORDING

      if (counter % (measure_note_cycles) < (measure_note_cycles / 4)) {
        if (counter % (eigth_note_cycles / 2) == 0) {
          poll_signal |= (1 << CPAD_DOWN); // tom
        }
      } else {
        if (counter % (eigth_note_cycles / 2) == 0) {
          poll_signal |= (1 << CPAD_LEFT); // tom
        }
      }
    }
    if (counter % (measure_note_cycles) == measure_note_cycles / 2) {
      poll_signal |= (1 << BUTTON_RIGHT); // crash!
    }
  }

  // tenth measure. stop loop
  else if (counter < 10*measure_note_cycles) {
    if (counter % (measure_note_cycles) == 0) {
      poll_signal |= (1 << CPAD_UP); // crash!
      poll_signal |= (1 << BUTTON_LEFT); // stop loop!
    }
  }

  else {
    counter = 0;
  }

  /*
  if (counter % quarter_note_cycles == 0) {
    poll_signal |= (1 << BUTTON_A); // just a bass drum
  }

  if (counter % (quarter_note_cycles * 2) == 0) {
    poll_signal |= (1 << BUTTON_B); // just a bass drum
  }*/

  return poll_signal;





      /*
      #define ARM_TIMER_LOD REG(0x2000B400)  999
      #define ARM_TIMER_VAL REG(0x2000B404)  between 0 and 999 (changes
      #define ARM_TIMER_CTL REG(0x2000B408)  4063394
      #define ARM_TIMER_CLI REG(0x2000B40C)  1414353473
      #define ARM_TIMER_RIS REG(0x2000B410)  0
      #define ARM_TIMER_MIS REG(0x2000B414)  0
      #define ARM_TIMER_RLD REG(0x2000B418)  999
      #define ARM_TIMER_DIV REG(0x2000B41C)  249
      #define ARM_TIMER_CNT REG(0x2000B420)  0




      // SYSTIMER_CLO
      // likely in us
      //mem = 0x20003004;
      //ra = GET32(mem);
      //chprintf((BaseSequentialStream*)&SD1, "SYSTIMER_CLO %d\r\n", (ra - old));
      //old = ra;


      mem = 0x2000B400;
      for (int l = 0; l < 9; l++) {
        ra = GET32(mem);
        chprintf((BaseSequentialStream *)&SD1, "ARM_TIMER_CNT %d\r\n", ra);
        mem += 0x04;
      }*/


      /*
      // event detect
      mem = 0x2000B200;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "IRQ basic pending %d\r\n", (ra));
      // event detect
      mem = 0x2000B20C;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "FIQ control%d\r\n", (ra));
      // event detect
      mem = 0x2000B210;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "Enable IRQs1 %d\r\n", (ra));

      // bits set are: 29 and 9

      // event detect
      mem = 0x2000B214;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "Enable IRQs2 %d\r\n", (ra));
      // event detect
      mem = 0x2000B218;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "Enable Basic IRQs %d\r\n", (ra));

      mem = 0x2000B21C;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "Disable IRQs1 %d\r\n", (ra));
      // event detect
      mem = 0x2000B220;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "Disable IRQs2 %d\r\n", (ra));
      // event detect
      mem = 0x2000B224;
      ra = GET32(mem);
      chprintf((BaseSequentialStream *)&SD1, "Disable Basic IRQs %d\r\n", (ra));



      */
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
  chprintf((BaseSequentialStream *)&SD1, "FINAL DRUM64!!\r\n");




  /*
   * I2C initialization.
   * init_system_I2C writes the write address, etc.
   */
  I2CConfig i2cConfig;
  i2cConfig.ic_speed = 1000000; // manifests as 1.6 MHz
  i2cStart(&I2C0, &i2cConfig);
  init_system_I2C();

  //chThdSleepMilliseconds(1000);
  /*
   *  initialize LED (pin 8)
   */
  palSetPadMode(ONBOARD_LED_PORT, ONBOARD_LED_PAD, PAL_MODE_OUTPUT);


  /*
   *  set GPIO pin 20 to INPUT. Set Interrupts
   */

  palSetPadMode(GPIO26_PORT, GPIO26_PAD, PAL_MODE_INPUT);

  // set Asynchronous rising edge Detect for pin 26
  mem = 0x2020007C; // ASYNC    //mem = 0x2020004C; // SYNC
  ra = GET32(mem);
  ra |= (1 << 26);
  PUT32(mem, ra);

  // set Interrupts on GPIO_INT[0]
  mem = 0x2000B214;
  ra = GET32(mem);
  ra |= (1 << 17); // bit 17 + i is for GPIO_INT[i], for i = 0, 1, 2, 3
  PUT32(mem, ra);


  /*
   * Serial General Purpose Timer (GPT) #1 initialization.
   */



  /*
   *  Initialize PWM
   */
  PWMConfig pwmConfig;
  pwmConfig.period = 34; // ~4us // 240kHz. we want 280kHz

  pwmStart(&PWMD1, &pwmConfig);
  pwmEnableChannel(&PWMD1, 0, 0); // 3 high 1 low ... I think it's default high.

  mem = PWMCTL; // PWM register
  ra = GET32(mem);
  ra |= ((1 << 7) | (1 << 4) | (1 << 5) | (1 << 2));
  // enable MSNEN of PWM register
  // flip polarity so that it's low to high (as opposed to high to low)
  // use FIFO
  // repeats last FIFO data
  // do NOT use this //ra |= (1 << 3);   // silenced to HIGH
  PUT32(mem, ra);

  // set duty cycle to 0%
  mem = PWMCTL + 0x18;
  PUT32(mem, 0);

  // wait 500 milliseconds to boot
  chThdSleepMilliseconds(500);


  while (1) {
    mem = PWMCTL + 0x18;
    PUT32(mem, 0);
    chThdSleepMilliseconds(200);

    controller_state = read_poll_signal();

    chThdSleepMilliseconds(200);
    send_poll_signal();
  }


  // commented out from here on out!
  // LOAD SAMPLES from data onto samples[i][t]
  load_samples();

  // all structs
  // initialize current controller state
  // set all to zero ..?
  CurrentState cstate = {0};
  //cstate.fresh = 0;
  DrumCounters dcounters = {0};
  LoopState lstate = {0};

  // DEBUGGING: START WITH ALL DRUMS PLAYING
  for (int i = 0; i < 8; i++) {
    dcounters.drum_playing[i] = 1;
  }
  // start with ride cymbol!
  //dcounters.drum_playing[6] = 1;
  //dcounters.drum_counter[6] = 0;



  /*
   * Serial General Purpose Timer (GPT) #1 initialization.
   * write new sample to I2C periodically at Sample Rate
   */
  GPTConfig gptConfig1;
  gptConfig1.callback = write_sample;
  gptStart(&GPTD2, &gptConfig1);
  // gptStartContinuous(&GPTD2, 50); // 20kHz.
  // 10 kHz audio.
  gptStartContinuous(&GPTD2, 1000000 / SAMPLERATE);

  /*
  while (1) {

    if (((counter - audio_buffer_counter) % AUDIO_BUFFER_SIZE == 0) ||
        (((counter + 1) - audio_buffer_counter) % AUDIO_BUFFER_SIZE == 0)) {
      continue;
    }

    palClearPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    palSetPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);


    counter++;


    if ((counter - POLL_READ) % POLL_PERIOD == 0) {
      // poll to get controller state
      send_poll_signal();
      cstate.fresh = 0;
    }
    if (counter % POLL_PERIOD == 0) {
      controller_state = read_poll_signal();

      // DEBUG so read_poll_signal works in generating a fake signal!
      //if (controller_state != 0) {
      //  chprintf((BaseSequentialStream *)&SD1, "Controller %d\r\n", controller_state);
      //}

      // update state
      update_state(&cstate, controller_state, old_controller_state);
      old_controller_state = controller_state;

      // update l
      update_loop_state(&lstate, &cstate);
    }
    else {
      cstate.fresh = 0;
    }



    // update drum counters
    update_drum_counters(&dcounters, &cstate, &lstate);




    // write audio buffer data.
    // PAUSE if buffer is too large ...

    // debugging: works!
    //audio_buffer[counter % AUDIO_BUFFER_SIZE] = data[counter];
    //audio_buffer[counter % AUDIO_BUFFER_SIZE] = samples[0][counter];
    audio_buffer[counter % AUDIO_BUFFER_SIZE] = generate_sample(&dcounters);

    continue;
  }*/


  return 0;
}
