#ederwander
import pyaudio
import numpy as np
import wave
import time


import Tkinter # for GUI interface ...

p = pyaudio.PyAudio()
print(p.get_device_info_by_index(0)['defaultSampleRate'])




n_sounds = 4

snare = wave.open("Snare-707-b.wav", mode=None)
kick = wave.open("Kick-707-b.wav", mode=None)
hihat = wave.open("Hihat-707-Open.wav", mode = None)
hihatc = wave.open("Hihat-707-Closed.wav", mode = None)

drums = [kick, snare, hihat, hihatc]

pressed = [False, False, False, False]
start_pressed = False
stop_pressed = False

labels = ('kick','snare', 'hihat open', 'hihat closed')

# buttons: Loop start / stop. Delete last loop.
# buttons: snare, kick, hihat, hihatcs
# sliders: volume for each of the sounds

chunk = 4500
#90 # samples per chunk #purposefully divisible by 3
# buffer size in samples
# 1024 samples / (96000 kHz) = 10 ms
# longest loop = 10 seconds = 1000 samples
loop_length = 10000
loops = np.zeros((loop_length, 4))
loops_recent = np.zeros((loop_length, 4))
#loops_recent[::8, :] = 100.0
loop_length = 0
loop_counter = 0
loop_initiated = False
loop_started = False


FORMAT = pyaudio.paInt16 # 16 bit audio
CHANNELS = 1
RATE = 96000

stream = p.open(format = FORMAT,
                channels = CHANNELS,
                rate = RATE,
                input = True,
                output = True,
                frames_per_buffer = chunk)

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

Tkinter.Button(root, text="Loop", width=7, command=set_start).grid(row=1,column=1)
#Tkinter.Button(root, text="Erase", width=7, command=set_stop).grid(row=1,column=2)
txtbox = Tkinter.Entry(root, textvariable = mystring).grid(row=1,column=2)
#txtbox.pack()


for n in range(n_sounds):
    amplitudes.append(Tkinter.DoubleVar())
    buttons.append(Tkinter.Button(root, text=labels[n], width=7, command=set_pressed(n)))
    buttons[n].grid(row=1,column=n+3)
    scales.append(Tkinter.Scale(root, variable = amplitudes[n], from_=100, to=0, resolution=0.1))
    scales[n].set(50)
    scales[n].grid(row=2,column=n+3)



n = 0


current_amps = [50.0, 50.0, 50.0, 50.0]


old_time = time.clock()
#main loop after everything is loaded
while(True):
    new_time = time.clock()
    print(new_time - old_time)
    old_time = new_time
    root.update()
    n += 1

    s = mystring.get()

    if " " in s:
        print("Start")
        global loop_initiated
        global loop_started
        start_pressed = True
        if not loop_initiated:
            loop_initiated = True
        elif loop_initiated:
            loop_started = True
    for j in range(n_sounds):
        if str(j+1) in s:
            pressed[j] = True
            drums[j].rewind()
    if "z" in s:
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


    mystring.set("")



    if loop_initiated and (not loop_started):
        print(loop_length)
        for i in range(n_sounds):
            if pressed[i]:
                loops_recent[loop_length, i] = scales[i].get()
                print(scales[i].get())
        loop_length += 1

    elif loop_started:
        loops[loop_counter, :] += loops_recent[loop_counter, :]
        loops_recent[loop_counter, :] *= 0 # resets loops_recent

        for i in range(n_sounds):
            if pressed[i]:
                loops_recent[loop_counter, i] = scales[i].get()
                drums[i].rewind()
                current_amps[i] = loops_recent[loop_counter, i]
            elif loops[loop_counter, i] != 0:
                drums[i].rewind()
                current_amps[i] = loops[loop_counter, i]

        loop_counter += 1
        if loop_counter == loop_length:
            loop_counter = 0



    #a, p = gen_params()
    data = np.zeros(chunk)

    for i in range(n_sounds):
        s = drums[i].readframes(chunk / 3)
        d = wave.struct.unpack("%dh"%(len(s) / 2), s)
        d = np.array(d)
        d = d.astype("float")
        d *= current_amps[i] / (100.0 * n_sounds)
        if len(d) < len(data):
            d = np.pad(d, (0, len(data) - len(d)), 'constant')
        data += d
    data = data.astype("int")
    #print(data)

    signal = wave.struct.pack("%dh"%(len(data)), *list(data))
    stream.write(signal)

    for i in range(n_sounds):
        pressed[i] = False



stream.stop_stream()
stream.close()
p.terminate()
