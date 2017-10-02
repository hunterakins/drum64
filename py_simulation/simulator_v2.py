# rebuilt to match what the n64 controller is actually like
import pyaudio
import numpy as np
import wave
import time
from DrumState import DrumState


import Tkinter # for GUI interface ...

p = pyaudio.PyAudio()
print(p.get_device_info_by_index(0)['defaultSampleRate'])



# BUFFER SIZE (must be divisible by 3 ... for some reason)
buffer_size = 100


#
# GLOBAL CONSTANTS!!!
#

#BUTTON MAPS
a_button=0; b_button=1; z_button=2; start_button=3;
dpad_up=4; dpad_down=5; dpad_left=6; dpad_right=7;
left_button=10; right_button=11;
c_up=12; c_down=13; c_left=14; c_right=15;
# analog X-axis is 16-23; analog Y-zxis = 24-31;
# maps keyboards to buttons
button_map = {'m': a_button, 'n': b_button, 'z': z_button, ' ': start_button,
    'w': dpad_up, 's': dpad_down, 'a': dpad_left, 'd': dpad_right,
    '[': left_button, ']': right_button,
    'i': c_up, 'k': c_down, 'j': c_left, 'l': c_right
    }


# sounds
# this LOADS THE SOUNDS
directory = 'sounds/'
snare = wave.open(directory + "Snare-707-b.wav", mode=None)
kick = wave.open(directory + "Kick-707-b.wav", mode=None)
tomlow = wave.open(directory + "Tom-707-Low.wav", mode = None)
tomhi = wave.open(directory + "Tom-707-Hi.wav", mode = None)
hihat = wave.open(directory + "Hihat-707-Open.wav", mode = None)
hihatc = wave.open(directory + "Hihat-707-Closed.wav", mode = None)
ride = wave.open(directory + "Ride-707.wav", mode = None)

drums = [kick, snare, tomhi, tomlow, hihat, hihatc, ride]
buttoms_drums_map = [a_button, b_button, c_down, c_left, c_up, c_right, right_button]
#{a_button: kick, b_button: snare, c_down: tomhi, c_left: tomlow, c_up: hihat, c_right: hihatc, right_button: ride}




FORMAT = pyaudio.paInt16 # 16 bit audio
CHANNELS = 1
RATE = 96000

stream = p.open(format = FORMAT,
                channels = CHANNELS,
                rate = RATE,
                input = True,
                output = True,
                frames_per_buffer = buffer_size)

# this initializes tkinter
root = Tkinter.Tk()


amplitudes = []
buttons = []
scales = []


def set_pressed(n):
    "returns function that sets the n'th pressed to True"
    global pressed
    def s():
        pressed[n] = True
        drums[n].rewind()
        return
    return s

def set_start():
    print("Start")
    global loop_initiated
    global loop_started
    start_pressed = True
    if not loop_initiated:
        loop_initiated = True
    elif loop_initiated:
        loop_started = True
    return

def set_stop():
    print("Stop")
    global loops_recent
    global loops
    global loop_length
    global loop_initiated
    global loop_started
    stop_pressed = True
    loops_recent *= 0
    loops *= 0
    loop_length = 0
    loop_initiated = False
    loop_started = False
    return

mystring = Tkinter.StringVar()

#Tkinter.Button(root, text="Loop", width=7, command=set_start).grid(row=1,column=1)
#Tkinter.Button(root, text="Erase", width=7, command=set_stop).grid(row=1,column=2)
txtbox = Tkinter.Entry(root, textvariable = mystring).grid(row=1,column=2)


amp = Tkinter.IntVar()
filt = Tkinter.IntVar()
AnalogY = Tkinter.Scale(root, variable = amp, from_=255, to=0, resolution=1)
AnalogY.grid(row = 2, column = 1)
AnalogY.set(128)
Tkinter.Label(root, text="amplitude").grid(row = 3, column = 1)
AnalogX = Tkinter.Scale(root, variable = filt, from_=255, to=0, resolution=1)
AnalogX.grid(row = 2, column = 3)
AnalogX.set(128)
Tkinter.Label(root, text="filter").grid(row = 3, column = 3)
#txtbox.pack()

"""
for n in range(n_sounds):
    amplitudes.append(Tkinter.DoubleVar())
    buttons.append(Tkinter.Button(root, text=labels[n], width=7, command=set_pressed(n)))
    buttons[n].grid(row=1,column=n+3)
    scales.append(Tkinter.Scale(root, variable = amplitudes[n], from_=100, to=0, resolution=0.1))
    scales[n].set(50)
    scales[n].grid(row=2,column=n+3)
    """



n = 0


current_amps = [50.0, 50.0, 50.0, 50.0]




old_time = time.clock()


def button_state_from_string(s, AnalogY_val):
    "from string, returns the button state (32 bit array)"
    temp = AnalogY_val
    button_state = [0] * 32
    for key in button_map.keys():
        if key in s:
            button_state[button_map[key]] = 1
    for i in range(8):
        button_state[16 + i] = (temp % 2**(i+1) != 0)
        temp -= temp % 2**(i+1)
    return button_state

# write to buffer function??
def write_buffer(onsets, amplitudes): # onsets happen
    "returns a buffer, given currently: onsets (should we reset the sound?) and amplitudes"
    buff = np.zeros(buffer_size)

    for i in range(len(drums)):
        #print(i)
        #print(drums[i])
        if onsets[i]:       # rewind if onset
            drums[i].rewind()

        s = drums[i].readframes(buffer_size)

        d = list(wave.struct.unpack("%dh"%(len(s) / 2), s))
        d += [0] * (buffer_size - len(d))

        d = np.array(d)
        d = d.astype("float")
        d *= amplitudes[i] / (len(drums))

        buff += d

    buff = buff.astype("int")

    return buff






state = DrumState()


#main loop after everything is loaded
while(True):
    n += 1

    # sees how much time between loops.
    new_time = time.clock()
    #print(new_time - old_time) #
    old_time = new_time

    root.update()

    button_state_current = button_state_from_string(mystring.get(), AnalogY.get())
    if n % 10 == 0:
        mystring.set("")
    state.set_button_state_current(button_state_current)

    state.update_state()

    onsets, amplitudes = state.get_onset_amplitudes()

    #print(amplitudes)

    state.update_loop()
    """print(onsets, amplitudes)
    print(state.button_state_onset)
    print(state.button_state_offset)
    print(state.button_state_current)
    print(state.button_state_held)"""




    buff = write_buffer(onsets, amplitudes) # this function should be able to happen at any time
    signal = wave.struct.pack("%dh"%(len(buff)), *list(buff))
    stream.write(signal)

    #a = raw_input()


    # for now, we will have the buffer play every buffer_size
    # for the actual product, though, the True loop will run every ~300 us
    # and the write Audio will be controlled by an interrupt at 1/44.1kHz ~= 22.7 us



stream.stop_stream()
stream.close()
p.terminate()
