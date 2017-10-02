import numpy as np
import scipy.io.wavfile
import scipy.signal
import time
import scipy.io.wavfile

filenames = ("0_kick", "1_snare", "2_tomhi", "3_tomlo", "4_hihatc", "5_hihato", "6_ride")

for filename in filenames:
    a = open(filename + ".h", 'w')

    samplerate, data = scipy.io.wavfile.read(filename + ".wav")


    #print(data)
    if (data.dtype != np.dtype('int16')):
        print("This behaved unexpectedly ...!")

    data = data.astype('int')

    if len(data.shape) == 2:
        data = (data[:, 0] + data[:, 1]) / 2

    if data[0] == 0:
        0/0
        print("too bad!")
        continue
    #print(data)
    data -= min(data)


    sample_divider = 4
    bit_depth = 12 # only 12 bits!
    bits_to_remove = 16 - bit_depth

    new = []
    for i in range(int(len(data) / sample_divider)):
        new.append(int(np.mean(data[sample_divider*i:sample_divider*(i+1)])))

    st = "const uint16_t data_" + filename + "[" + str(len(new)) + "] = {"
    for item in new:
        st += hex(item >> bits_to_remove) + ","
        #print(bin(item >> bits_to_remove))
    st += "};"

    a.write(st)
    a.close()
    #print(st)
    #input()
