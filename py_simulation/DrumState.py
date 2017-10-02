# Drum State class
import numpy as np


class DrumState:

    max_loop_length = 10000 # make it something like 30 seconds ...
    num_drums = 7 # number of drums. = number of buttons available on right hand.


    #BUTTON MAPS
    a_button=0; b_button=1; z_button=2; start_button=3;
    dpad_up=4; dpad_down=5; dpad_left=6; dpad_right=7;
    left_button=10; right_button=11;
    c_up=12; c_down=13; c_left=14; c_right=15;

    drum_keys = [a_button, b_button, c_down, c_left, c_up, c_right, right_button]


    def __init__(self):
        self.button_state_current = [False] * 32 #boolean what the current button state is
        self.button_state_held = [0] * 32 #how long the button has been held #integer array
        self.button_state_unheld = [0] * 32 #how long the button has been held #integer array


        self.button_state_onset = [False] * 32 #boolean what the current button state is
        self.button_state_offset = [False] * 32 # boolean if offset just occurred

        # current drum onsets and amplitudes
        self.sound_onsets = [False] * DrumState.num_drums
        self.sound_amplitude = 1.0
        self.sound_amplitudes = [1.0] * DrumState.num_drums

        # historic drum onsets and amplitudes
        # sparse -- mostly zeros, until onset
        self.loop_amplitudes = np.zeros((DrumState.max_loop_length, DrumState.num_drums))



        self.loop_init = False
        self.loop_started = False
        self.loop_recording = False
        self.loop_length = 1 # Loop length has not yet been set
        self.loop_counter = 0 # counter that sets when

        # mute if left button is pressed!
        self.loop_muted = False

    def update_state(self):
        "This updates the state based on most recent button states"

        self.sound_onsets = [False] * DrumState.num_drums

        # writes current sound amplitude

        self.sound_amplitude = np.inner(self.button_state_current[16:24], 2**np.arange(8)) / 256. # anlog stick data somehow
        #print(self.button_state_current[16:24])
        #print(self.sound_amplitude)

        # writes current sound onsets for drum keys
        for i in range(len(DrumState.drum_keys)):
            if self.button_state_onset[DrumState.drum_keys[i]]:
                #print(DrumState.drum_keys[i], "is on")
                #print(self.sound_onsets)
                self.sound_onsets[i] = True
                self.sound_amplitudes[i] = self.sound_amplitude
                #print(self.sound_amplitude)


    def update_loop(self):
        "updates loop"

        # if recording
        if self.loop_recording:
            for i in range(len(self.sound_onsets)):
                if self.sound_onsets[i]:   # for every new sound onset:
                    #print(i, "is on (sound)")
                    #print(self.sound_onsets)
                    self.loop_amplitudes[self.loop_counter, i] = self.sound_amplitude




        if self.loop_init and (not self.loop_started): # if z is pressed for JUST the first time
            assert self.loop_length <= DrumState.max_loop_length
            self.loop_counter += 1
            self.loop_length += 1

        # if already looping:
        elif self.loop_started:
            # increase loop counter, start from beginning if exceeds loop_length
            self.loop_counter += 1
            if self.loop_counter == self.loop_length:
                self.loop_counter = 0

            for i in range(len(self.sound_onsets)):
                if (self.loop_amplitudes[self.loop_counter, i] != 0):
                    # UPDATE current amplitudes to loop amplitudes for those with onset:
                    self.sound_amplitudes[i] = self.loop_amplitudes[self.loop_counter, i]





        # Z BUTTON ONSET
        if self.button_state_onset[DrumState.z_button]:
            z_button_Onset()

        # L BUTTON MONSET, OFFSET
        if self.button_state_onset[DrumState.left_button]:
            left_button_Onset()

        #if self.button_state_offset[DrumState.left_button] and self.button_state_held[DrumState.left_button] > 10:
        #    print("unmuted")
        #    self.loop_muted = False


    def z_button_Onset(unheld = 0):
        if (not self.loop_init): #1st z-press: INITIALIZES start of loop for the first time
            self.loop_init = True
            self.loop_recording = True

        elif self.loop_init and (not self.loop_started): #2nd z-press: sets the end of the loop
            self.loop_started = True
            self.loop_recording = True

        else: # 3rd and subsequent z-press: toggles on and off z-recording
            self.loop_recording = not self.loop_recording


    def left_button_Onset(unheld = 0):
        print(not self.loop_muted, "muted")
        self.loop_muted = not self.loop_muted #True




    def get_onset_amplitudes(self):
        "returns the onsets, and amplitudes of current key presses, given loops?"
        onsets, amplitudes = self.sound_onsets[:], self.sound_amplitudes[:]
        if self.loop_started and not(self.loop_muted):
            for i in range(DrumState.num_drums):
                if (not self.sound_onsets[i]) and (self.loop_amplitudes[self.loop_counter, i] != 0):
                    # if (and only if) new onset in loop
                    onsets[i] = self.loop_amplitudes[self.loop_counter, i] != 0
                    amplitudes[i] = self.loop_amplitudes[self.loop_counter, i]
        return onsets, amplitudes


    def set_button_state_current(self, button_state_current):
        # update previous, first of all:
        for i in range(32):
            if self.button_state_current[i]:
                self.button_state_held[i] += 1
                self.button_state_unheld[i] = 0
            else:
                self.button_state_held[i] = 0
                self.button_state_unheld[i] += 1


        # set new button state
        self.button_state_current = button_state_current

        # resets what the new onset/offset state is
        self.button_state_onset = [False] * 32 #boolean what the current button state is
        self.button_state_offset = [False] * 32 # boolean if offset just occurred
        for i in range(32):
            if self.button_state_current[i] and (self.button_state_held[i] == 0):
                self.button_state_onset[i] = True # onset just occurred
            elif (not self.button_state_current[i]) and (self.button_state_held[i] > 0):
                self.button_state_offset[i] = True
