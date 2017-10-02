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


// structs CurrentState, LoopState, and DrumCounters
#include "current_state.h"


// sample rate, number of samples-per-poll, number of samples-to-wait-before-read
#define SAMPLERATE 11025  // in Hz
#define POLL_PERIOD 30 // number of samples per poll signal. goal: every 100us
#define POLL_READ   5 // number of samples to wait to read N64 poll output
#define POLL_READ_INTERRUPT_HOP   1 // number of samples to HOP as to denoise the RASP-Pi interrupt state read. should be not needed when attiny is used for polling


// audio buffer that is written to DAC
#define AUDIO_BUFFER_SIZE 128 // length of audio buffer in samples. 128 seems to work ok
volatile uint16_t audio_buffer[AUDIO_BUFFER_SIZE];
volatile uint16_t audio_buffer_counter;

// datafiles of sounds, sampled at SAMPLERATE. Each sample is uint16_t
#include "0_kick.h"
#include "1_snare.h"
#include "2_tomhi.h"
#include "3_tomlo.h"
#include "4_hihatc.h"
#include "5_hihato.h"
#include "6_ride.h"

// array length of each of the data files
uint32_t sample_sizes[8] = {1258, 2617, 16372, 17109, 1619, 3926, 35325, 0};

// maximum length of samples: seconds * samplerate
#define MAX_SAMPLE_LENGTH 4*SAMPLERATE
volatile uint16_t samples[8][MAX_SAMPLE_LENGTH];

// memory addresses of registers for baremetal
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

// NOOP!  does nothing!
#define noop

//
uint32_t controller_state = 0; // is polling done?
uint32_t controller_state_old = 0;
uint32_t controller_state_old2 = 0;
uint32_t controller_state_old3 = 0;



// for main loop:
uint8_t sample_done = 0;
uint16_t incrementer_old = 0; // old incree time
uint32_t counter = 10; //



/*
 * Loads samples from 0_kick.h, 1_snare.h, etc. onto samples[i][j]
 */
void load_samples(void) {
  volatile uint16_t sample;
  for (unsigned int i = 0; i < 8; i++) { // cycles between 1 - 8
    for (unsigned int j = 0; j < MAX_SAMPLE_LENGTH; j++) {
      if (j < sample_sizes[i]) {
        if (i == 0) {
          sample = data_0_kick[j]; // from 0_kick.h
        } else if (i == 1) {
          sample = data_1_snare[j]; // from 1_snare.h
        } else if (i == 2) {
          sample = data_2_tomhi[j]; // from 2_tomhi.h
        } else if (i == 3) {
          sample = data_3_tomlo[j]; // from 3_tomlo.h
        } else if (i == 4) {
          sample = data_4_hihatc[j]; // from 4_hihatc.h
          //sample = 0x7FF + ((data_4_hihatc[j] - 0x7FF) >> 2); //HACKS HIHAT IS TOO LOUD
        } else if (i == 5) {
          sample = data_5_hihato[j]; // from 5_hihato.h
        } else if (i == 6) {
          sample = data_6_ride[j]; // from 6_ride.h
        }
        samples[i][j] = sample;
      } else {
        sample = 0x7FF; // load 0 into sample
        samples[i][j] = sample;
      }
      // samples[i][j] = sample;
    }
  }
}

/*
 * Uses i2cMasterTransmitTimeout from Chibi-OS's drivers to initialize system.
 * Likely not necessary for future iterations
 *
 * writes 0 to DAC
 */
void init_system_I2C(void) {
  static const uint8_t mcp4725_address = 0x60;
  uint16_t level = 0xffff; // sets to zero
  uint8_t request[2];
  request[0] = (level >> 8) & 0xff;
  request[1] = level & 0xff;

  // try to not rely on i2cMasterTransmitTimeout for future iterations ...
  i2cAcquireBus(&I2C0);
  i2cMasterTransmitTimeout(
    &I2C0, mcp4725_address, request, 2,
    NULL, 0, TIME_INFINITE); //MS2ST(100));
  i2cReleaseBus(&I2C0);
}

/*
 * Writes sample via I2C to DAC.
 * AND ... sends Poll Signal to N64 controller if poll_period is right
 */
void write_sample(GPTDriver *gptp) { // takes about 3us of computation

  volatile uint32_t mem = 0;
  volatile uint32_t ra = 0;


  UNUSED(gptp);
  chSysLockFromIsr();

  GPTD2.period = (1000000 / SAMPLERATE);

  if ((audio_buffer_counter + 1 - POLL_READ) % POLL_PERIOD == 0) {
    // sample RIGHT BEFORE polling. sets hop as to not disturb interrupt collection
    palClearPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    palSetPad(ONBOARD_LED_PORT, ONBOARD_LED_PAD);
    GPTD2.period = (1 + POLL_READ_INTERRUPT_HOP) * (1000000 / SAMPLERATE);
  }
  else if ((audio_buffer_counter - POLL_READ) % POLL_PERIOD == 0) {
    // poll to get controller state
    send_poll_signal_PWM();
    //
    // audio_buffer_counter = audio_buffer_counter + POLL_READ_INTERRUPT_HOP; // increments audio_buffer_counter to compensate for skipped
  }

  // STEP 1: write buffer (2/2)
  mem = BSC0 + 0x10; // put into fifo
  ra = (audio_buffer[audio_buffer_counter % AUDIO_BUFFER_SIZE] >> 4) & 0xff;
  // changed from ra = audio_buffer[audio_buffer_counter] & 0xff;
  // and ra = (audio_buffer[(audio_buffer_counter+1) % AUDIO_BUFFER_SIZE] >> 8) & 0xff;
  // so that overflow doesn't happen!
  PUT32(mem, ra);

  // STEP 2: set bits to WRITE
  mem = BSC0; // STATUS
  ra = GET32(mem);
  ra |= (BSC_INTT | BSC_INTD | START_WRITE);;
  PUT32(mem, ra);

  // STEP 3: write buffer (1/2)
  mem = BSC0 + 0x10; // put into fifo
  ra = (audio_buffer[(audio_buffer_counter+1) % AUDIO_BUFFER_SIZE] >> 12) & 0xff;
  PUT32(mem, ra);

  // increment audio buffer
  audio_buffer_counter++;

  /*if ((audio_buffer_counter - POLL_READ) % POLL_PERIOD == 0) {
    audio_buffer_counter = audio_buffer_counter + POLL_READ_INTERRUPT_HOP; // increments audio_buffer_counter to compensate for skipped
  }*/
  chSysUnlockFromIsr();
}

/*
 * Writes sample via I2C to DAC.
 * AND ... sends Poll Signal to N64 controller if poll_period is right
 */
void update_state(CurrentState *cstate, uint32_t controller_state, uint32_t controller_state_old, uint32_t controller_state_old2, uint32_t controller_state_old3) {
  //chprintf((BaseSequentialStream *)&SD1, "CONTROLLER STATE %d\r\n", (controller_state));
  cstate->amplitude_X = 128 + ((controller_state >> XAXIS_START) & 0xff);
  cstate->amplitude_Y = 128 + ((controller_state >> YAXIS_START) & 0xff);
  cstate->fresh = 1;

  uint8_t drum_button_onset = 0;
  uint8_t loop_onset = 0;
  uint8_t mute_press = 0;
  uint8_t loop_kill_press = 0;

  // onset if controller_state, ..._old, ..._old2 is {110}
  uint16_t onsets = (controller_state & (controller_state_old & ~(controller_state_old2) & ~(controller_state_old3)));
  // offset if controller_state, ..._old, ..._old2 is {001}
  uint16_t offsets = (~(controller_state) & (~(controller_state_old) & controller_state_old2 & controller_state_old3));
  // pressed if controller_state, ..._old, ..._old2 is {111}
  uint16_t pressed = (controller_state & (controller_state_old & controller_state_old2 & controller_state_old3));


  for (int i = 0; i < 16; i++) {
    if (onsets & (1 << i)) {

      if (i == BUTTON_A)       // drums should be played.
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

      else if (i == BUTTON_LEFT)  // if left button: KILL LOOP
        loop_kill_press = 1;

      else if (i == BUTTON_START) // Start button onsets loop. Switch with Z
        loop_onset = 1;
    }

    // CONTINUAL PRESS
    else if (pressed & (1 << i)) {
      if (i == BUTTON_Z) // z button mutes. switch with Start
        mute_press = 1;
    }

    // OFFSET
    else if (offsets & (1 << i)) {
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
     // if loop hasn't even started yet!
     if (lstate->loop_started == 0) {
       chprintf((BaseSequentialStream *)&SD1, "LOOP STARTED for first time\r\n");
       if (lstate->loop_length == 0) {
         lstate->loop_started = 1;
         lstate->loop_recording = 1;
       } else { // paused and now resumed
         lstate->loop_started = 1;
       }
     }
     // if loop was pressed just a single time
     else if (lstate->loop_length == 0) {
       chprintf((BaseSequentialStream *)&SD1, "LOOP DONE\r\n");
       lstate->loop_length = lstate->loop_counter+1;
       //lstate->loop_counter = 0; // reset loop_counter to zero!
       lstate->loop_recording = 0; // no longer recording
     } else {
       chprintf((BaseSequentialStream *)&SD1, "LOOP TOGGLE\r\n");
       // toggling between recording and/or not recording
       if (lstate->loop_recording) {
         lstate->loop_recording = 0;
       } else {
         lstate->loop_recording = 1;
       }
     }
   }

   if (loop_kill_press) { // reset everything to 0
     chprintf((BaseSequentialStream *)&SD1, "KILLED\r\n");
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
 * Increments the drum counters, triggers drums if current_state / loop_state dictates it
 */
void update_drum_counters(DrumCounters *dcounters, CurrentState *cstate, LoopState *lstate) {
  if (cstate->fresh != 0) { // Only update drum_counters if read recently occurred
    cstate->fresh = 0;
    // if button ONSET or LOOP ONSET
    for (int i = 0; i < 8; i++) {
      // button ONSET TRUMPS!
      if (cstate->drum_button_onset & (1 << i)) {
        dcounters->drum_playing[i] = 1;
        dcounters->drum_counter[i] = 0;
        dcounters->drum_amplitude_X[i] = cstate->amplitude_X;
        dcounters->drum_amplitude_Y[i] = cstate->amplitude_Y;
      }
      // if there is a trigger from a loop
      else if ((lstate->loop_amplitude_X)[i][lstate->loop_counter] != 0) {
        if (cstate->mute_press != 1) { // is NOT MUTED
          dcounters->drum_playing[i] = 1;
          dcounters->drum_counter[i] = 0;
          dcounters->drum_amplitude_X[i] = lstate->loop_amplitude_X[i][lstate->loop_counter];
          dcounters->drum_amplitude_Y[i] = lstate->loop_amplitude_Y[i][lstate->loop_counter];
        }
      }
    }
  }

  // increment counter if drum is playing, and stop it if counter reaches end
  for (int i = 0; i < 8; i++) {
    if (dcounters->drum_playing[i]) {
      (dcounters->drum_counter[i])++;
      //chprintf((BaseSequentialStream *)&SD1, "Drum Counter %d\r\n", (dcounters->drum_counter)[i]++);

      if ((dcounters->drum_counter)[i] == MAX_SAMPLE_LENGTH) {
        dcounters->drum_playing[i] = 0;
        dcounters->drum_counter[i] = 0;
      }
    }
  }
}

/*
 * Produces the current sample, from the drum counter stuct.
 */
uint16_t generate_sample(DrumCounters *dcounters) {
  uint16_t instant_sample = 0x7FFF; // Zero. (
  for (int i = 0; i < 8; i++) {
    if (dcounters->drum_playing[i]) { // if
      //if (dcounters->drum_counter[i] == 1)
      //  chprintf((BaseSequentialStream *)&SD1, "DRUM ONSET %d\r\n", i);

      if (i == 4) { // HACK: decreases the hihat sound by a little bit ...
        instant_sample += (samples[i][dcounters->drum_counter[i]] - 0x7FF) >> 2;
      }
      else {
        instant_sample += samples[i][dcounters->drum_counter[i]];
        instant_sample -= 0x7FF;
      }

    }
  }
  return instant_sample;
}


/*
 * Analyze Times. Analyzes the N64 interrupt counter times, from time array
 * returns button state
 *
 * if there is an error, return ~(0) = 11...1
*/
uint32_t analyze_times ( uint32_t *times , uint32_t increments_between) {
  uint32_t temp_controller_state = 0; // this is what is returned
  uint8_t j_counter = 32;
  uint8_t high = 0;
  uint8_t current_bit;

  // walk backwards
  // increments_between - 1, increments_between - 2, increments_between - 33 ...

  // first bit :
  int i = 0;
  current_bit = (times[increments_between - 2] - times[increments_between - 1]) % (1 << 8);
  if (current_bit < 7) { //(current_bit < 7) // low gap
    //chprintf((BaseSequentialStream *)&SD1, "0");
  } else if (current_bit > 8) {
    //chprintf((BaseSequentialStream *)&SD1, "2");
    high = 1;
  } else {
    //chprintf((BaseSequentialStream *)&SD1, "1");
  }
  if (high)
    temp_controller_state |= (1 << 31);


  // all other bits
  for (int i = 1; i < 33; i++) {
    current_bit = (times[increments_between - (i + 2)] - times[increments_between - (i + 1)]) % (1 << 8);
    if (current_bit < 6) { //(current_bit < 7)
      //chprintf((BaseSequentialStream *)&SD1, "0");
      if (high) {
        high = 0;
      } else {
        // broken
        //chprintf((BaseSequentialStream *)&SD1, "\r\n");
        return (~(0));
        //break;
      }
    }
    else if (current_bit > 8) {
      //chprintf((BaseSequentialStream *)&SD1, "2");
      if (~(high)) {
        high = 1;
      } else { // ERROR
        //chprintf((BaseSequentialStream *)&SD1, "\r\n");
        return (~(0));
        //break;
      }
    }
    else {
      //chprintf((BaseSequentialStream *)&SD1, "1");
    }

    if (high)
      temp_controller_state |= 1 << (32 - (i + 1));
  }
  //chprintf((BaseSequentialStream *)&SD1, "\r\n");


  return temp_controller_state;


  /*
  // DEBUG: prints out gaps
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
 * Sends poll signal over PWM
 *
 *
 */
void send_poll_signal_PWM(void) {
  volatile uint32_t mem = 0;
  volatile uint32_t ra = 0;

  // write to PWM FIFO
  mem = PWMCTL + 0x18;
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
 * CALL only after ample time after send_poll_signal is called (around 150us)
 * Accesses incrementer and times array from hal_lld.c
 * analyzes them to produce controller_state (uint32_t).
 */
uint32_t read_poll_signal(void) {

  // do analysis here
  uint32_t incrementer;
  uint32_t increments_between;
  uint32_t times[64];
  uint32_t temp_controller_state;

  incrementer = read_incrementer(); // from hal_lld.c ;
  increments_between = (incrementer - incrementer_old) % (1 << 8); // 43 is the magic number.
  // the % (1 <<8) = 256 is that sometimes the incrementer jumps by 2**16.
  // chprintf((BaseSequentialStream *)&SD1, "Number of pulse increments: %d\r\n", increments_between);//increments_between);
  incrementer_old = incrementer;


  read_times(times); // from hal_lld.c ;

  temp_controller_state = analyze_times(times, increments_between);
  return temp_controller_state;
}


/*
 * read_poll_signal_test_audio_algorithm
 * replaces read_poll_signal
 * instead of reading the actual incrementer, and times between Interrupts
 * generates signals to test the algorithm
 *
 */
uint32_t read_poll_signal_test_audio_algorithm(void) {

  uint32_t quarter_note_cycles = 7360; // approx 90bpm. important that it's divisible by 8?
  uint32_t eigth_note_cycles = quarter_note_cycles / 2;
  uint32_t measure_note_cycles = quarter_note_cycles * 4;
  uint32_t poll_signal = 0x7f7f0000; // mid amplitudes, nothing else
  // debug: return expected button presses

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

  return poll_signal;
}

/*
 * Application entry point.
 */
int main(void) {
  uint32_t mem = 0;
  uint32_t ra = 0;


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

  /*
  while (1) {
    mem = PWMCTL + 0x18;
    PUT32(mem, 0);
    chThdSleepMilliseconds(200);

    controller_state = read_poll_signal();
    // if controller state is bad ..?

    chThdSleepMilliseconds(200);
    send_poll_signal_PWM();
  }*/


  // commented out from here on out!
  // LOAD SAMPLES from data onto samples[i][t]
  load_samples();

  // all structs
  // initialize current controller state
  // set all to zero ..?
  CurrentState cstate = {0};
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
  //chprintf((BaseSequentialStream *)&SD1, "GPT period %d\r\n", GPTD2.period);//gptConfig1.callback


  while (1) {

    if (((counter - audio_buffer_counter) % AUDIO_BUFFER_SIZE == 0) ||
        (((counter + 1) - audio_buffer_counter) % AUDIO_BUFFER_SIZE == 0)) {
      continue;
    }



    counter++;


    if (audio_buffer_counter % POLL_PERIOD == 0) {
      controller_state = read_poll_signal();

      int check_sum = 0;
      for (int i = 0; i < 32; i++) {
        if (controller_state & (1 << i))
          check_sum++;
      }

      if ((controller_state != (~(0))) && (check_sum < 24)) { // if without bugs



        /*
        for (int i = 0; i < 32; i++) {
          if (controller_state & (1 << i)) {
            chprintf((BaseSequentialStream *)&SD1, "1");
          } else {
            chprintf((BaseSequentialStream *)&SD1, "0");
          }
        }

        chprintf((BaseSequentialStream *)&SD1, "\r\n");*/



        // DEBUG so read_poll_signal works in generating a fake signal!
        //if (controller_state != 0) {
        //  chprintf((BaseSequentialStream *)&SD1, "Controller %d\r\n", controller_state);
        //}

        // update state
        update_state(&cstate, controller_state, controller_state_old, controller_state_old2, controller_state_old3);
        controller_state_old3 = controller_state_old2;
        controller_state_old2 = controller_state_old;
        controller_state_old = controller_state;

        // update l
        update_loop_state(&lstate, &cstate);
      }
      //else {
      //  chprintf((BaseSequentialStream *)&SD1, "BUG\r\n");
      //}
    }



    // update drum counters
    update_drum_counters(&dcounters, &cstate, &lstate);


    // write audio buffer data.
    // PAUSE if buffer is too large ...

    // writes generated sample to audio buffer
    audio_buffer[counter % AUDIO_BUFFER_SIZE] = generate_sample(&dcounters);


    continue;
  }


  return 0;
}
