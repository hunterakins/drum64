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

uint16_t samples[8][MAX_SAMPLE_LENGTH];


#define PUT32(a,b) ((*(volatile unsigned int *)a) = (b))
#define GET32(a) (*(volatile unsigned int *) a)
#define BSC0  0x20205000
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028


#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define I2S_BASE                (BCM2708_PERI_BASE + 0x203000) /* GPIO controller */
#define CLOCK_BASE               (BCM2708_PERI_BASE + 0x101000) /* Clocks */

#define noop

#define BYTESPERBLOCK 4



#define SAMPLERATE 11025  // in Hz
#define POLL_PERIOD 8 // number of samples per poll

//static MUTEX_DECL(mutex1);
//static CONDVAR_DECL(cond1);



volatile uint32_t mem = 0;
volatile uint32_t ra = 0;
volatile uint32_t on;
volatile uint16_t state = 0;


uint32_t counter = 10;


#define AUDIO_BUFFER_SIZE 64

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
void load_samples() {
  for (unsigned int i = 0; i < 8; i++) {
    for (unsigned int j = 0; j < MAX_SAMPLE_LENGTH; j++) {
      if (j >= sample_sizes[i]) {
        samples[i][j] = 0x7FFFFF;
      } else {
        uint16_t sample;
        if (i == 0) {
          sample = data_0_kick[j];
        } else if (i == 1) {
          sample = data_1_snare[j];
        } else if (i == 2) {
          sample = data_2_tomhi[j];
        } else if (i == 3) {
          sample = data_3_tomlo[j];
        } else if (i == 4) {
          sample = data_4_hihatc[j];
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

void init_system() {
  uint16_t level = data[counter];
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


void interrupt_loop(GPTDriver *gptp) { // takes about 3us of computation
  UNUSED(gptp);
  chSysLockFromIsr();

  // write buffer (2/2)
  mem = BSC0 + 0x10; // put into fifo
  ra = audio_buffer[audio_buffer_counter] & 0xff;
  PUT32(mem, ra);

  // set bits to WRITe
  mem = BSC0; // STATUS
  ra = GET32(mem);
  ra |= (BSC_INTT | BSC_INTD | START_WRITE);;
  PUT32(mem, ra);

  // write buffer (1/2)
  mem = BSC0 + 0x10; // put into fifo
  ra = (audio_buffer[audio_buffer_counter+1] >> 8) & 0xff;
  PUT32(mem, ra);

  // increment audio buffer
  audio_buffer_counter++;
  if (audio_buffer_counter == AUDIO_BUFFER_SIZE)
    audio_buffer_counter = 0;

  chSysUnlockFromIsr();
}

void update_state(CurrentState *cstate, uint32_t controller_state, uint32_t old_controller_state) {
  // I have no idea what's going on!!!
  // write a class??
  cstate->amplitude_X = (controller_state << XAXIS_START) & 0xff;
  cstate->amplitude_Y = (controller_state << YAXIS_START) & 0xff;
  cstate->fresh = 1;

  uint8_t drum_button_onset = 0;
  uint8_t loop_onset = 0;
  uint8_t mute_press = 0;
  uint8_t loop_kill_press = 0;


  for (int i = 0; i < 16; i++) {
    uint8_t current = (controller_state & (1 << i)) >> i;
    uint8_t previous = (old_controller_state & (1 << i)) >> i;
    // ONSET ...
    if ((current) & ~(previous == 0)) {
      // loop started
      if (i == BUTTON_Z)
        loop_onset = 1;

      // drums should be played.
      else if (i == BUTTON_A)
        drum_button_onset |= (1 << 0);
      else if (i == BUTTON_B)
        drum_button_onset |= (1 << 1);
      else if (i == DPAD_DOWN)
        drum_button_onset |= (1 << 2);
      else if (i == DPAD_LEFT)
        drum_button_onset |= (1 << 3);
      else if (i == DPAD_RIGHT)
        drum_button_onset |= (1 << 4);
      else if (i == DPAD_UP)
        drum_button_onset |= (1 << 5);
      else if (i == BUTTON_RIGHT)
        drum_button_onset |= (1 << 6);
    }
    // CONTINUAL PRESS
    else if ((current) & (previous)) {
      // if start button: MUTE DRUMS
      if (i == BUTTON_START)
        mute_press = 1;
      // if left button: KILL LOOP
      else if (i == BUTTON_LEFT)
        loop_kill_press = 1;
    }
    // OFFSET
    else if (~(current == 0) & (previous)) {
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

   uint8_t drum_button_onset = drum_button_onset = drum_button_onset;
   uint8_t loop_onset                            = cstate->loop_onset;
   uint8_t loop_kill_press                       = cstate->loop_kill_press;
   uint8_t amplitude_X                           = cstate->amplitude_X;
   uint8_t amplitude_Y                           = cstate->amplitude_Y;



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
         if (drum_button_onset && (1 << i)) { // if onset of button i!
           (lstate->loop_amplitude_X)[i][lstate->loop_counter] = amplitude_X;
           (lstate->loop_amplitude_Y)[i][lstate->loop_counter] = amplitude_Y;
         }
       }
     }
   }


   if (loop_onset) {
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
     else if (lstate->loop_length = 0) {
       lstate->loop_length = lstate->loop_counter;
       lstate->loop_counter = 0; // reset loop_counter to zero!
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

   if (loop_kill_press) {
     lstate->loop_started;
     lstate->loop_recording;
     lstate->loop_length;
     lstate->loop_counter;
   }
 }

/*
 * INCREMENTS THE DRUM COUNTERS!!
 */
void update_drum_counters(DrumCounters *dcounters, CurrentState *cstate, LoopState *lstate) {
  if (cstate->fresh) { // FRESH!
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
      else if ((lstate->loop_amplitude_X)[i][lstate->loop_counter]) {
        dcounters->drum_playing[i] = 1;
        dcounters->drum_counter[i] = 0;
        dcounters->drum_amplitude_X[i] = lstate->loop_amplitude_X[i][lstate->loop_counter];
        dcounters->drum_amplitude_Y[i] = lstate->loop_amplitude_Y[i][lstate->loop_counter];
      }
    }
  }

  for (int i = 0; i < 8; i++) {
    // increment counter if drum is playing
    if (dcounters->drum_playing[i]) {
      (dcounters->drum_counter)[i]++;

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
  uint16_t sample = 0x7FFFFF; // smack dab in the middle between 0 and 0xffffff


  for (int i = 0; i < 8; i++) {
    if (dcounters->drum_playing[i]) {
      sample += samples[i][dcounters->drum_counter[i]];
      sample -= 0x7FFFFF;
    }
  }
  return sample;
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


  init_system();

  /*
   *  initialize LED (pin 8)
   */
  palSetPadMode(ONBOARD_LED_PORT, ONBOARD_LED_PAD, PAL_MODE_OUTPUT);

  // TRIES
  //gptStartContinuous(&GPTD2, 50); // This is more or less 20kHz. can be more precise.
  // 10 kHz audio.
  gptStartContinuous(&GPTD2, 10000000 / SAMPLERATE); // This is more or less 44.1kHz / 4. can be more precise.

  // LOAD SAMPLES!
  load_samples();

  // initialize current controller state
  CurrentState cstate;
  DrumCounters dcounters;
  LoopState lstate;

  // DEBUGGING: START WITH ALL DRUMS PLAYING
  for (int i = 0; i < 8; i++) {
    dcounters.drum_playing[i] = 1;
  }

  while (1) {

    if ((counter - audio_buffer_counter) % AUDIO_BUFFER_SIZE == 0) {
      continue;
    }

    palClearPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    palSetPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);


    counter++;


    /*
    if (counter % POLL_PERIOD == 0) {
      // poll to get controller state
      // update state
      update_state(&cstate, controller_state, old_controller_state);
      old_controller_state = controller_state;

      // update l
      update_loop_state(&lstate, &cstate);
    } else {
      cstate.fresh = 0;
    }
    */


    // update drum counters
    //update_drum_counters(&dcounters, &cstate, &lstate);

    // write audio buffer data.
    // PAUSE if buffer is too large ...
    audio_buffer[counter % AUDIO_BUFFER_SIZE] = data[counter];
    // samples[1][counter];
    //audio_buffer[counter % AUDIO_BUFFER_SIZE] = generate_sample(&dcounters);

    continue;
  }


  return 0;
}
