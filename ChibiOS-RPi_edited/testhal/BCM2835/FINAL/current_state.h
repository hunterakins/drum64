/*
 *  Header file which includes structs:
 *
 *  CurrentState, LoopState, and DrumCounters
 *
 */

 // definitions of BUTTONS
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


/**
 * @brief   Type of a structure representing the current button press state.
 *          stores the current relevent button onsets, offsets, and XY amplitudes
 *
*          Updated once every button press
 */
typedef struct CurrentState CurrentState;

struct CurrentState {
  uint8_t amplitude_X;
  uint8_t amplitude_Y;
  uint8_t drum_button_onset;
  uint8_t loop_onset;
  uint8_t mute_press;
  uint8_t loop_kill_press;
  volatile uint8_t fresh; // whether it's freshly updat'd!
};


/**
 * @brief   Type of a structure representing the Loop State
 *          stores whether loop has started, is recording, length, and counter
 *          as well as the saved loop_amplitudes for each of the presses
 *
 *          Updated once every button press
 */
typedef struct LoopState LoopState;

#define MAXLOOPTIME 2 // in seconds / poll_period
#define SAMPLERATE 11025  // in Hz


struct LoopState {
  uint8_t  loop_started;
  uint8_t  loop_recording;
  uint32_t loop_length;
  uint32_t loop_counter;
  uint8_t  loop_amplitude_X[8][SAMPLERATE*MAXLOOPTIME]; // if loop_amplitude_X[i][j] is NOT zero means an onset.
  uint8_t  loop_amplitude_Y[8][SAMPLERATE*MAXLOOPTIME];
};



/**
 * @brief   Type of a structure representing the counters of the DRUMS
 *          The current amplitude, whether they're playing, and which sample to
 *          load next into the buffer
 *
 *          Updated once every sample
 */
typedef struct DrumCounters DrumCounters;

struct DrumCounters {
  uint8_t drum_playing[8];
  uint32_t drum_counter[8];
  uint8_t drum_amplitude_X[8];
  uint8_t drum_amplitude_Y[8];
};
