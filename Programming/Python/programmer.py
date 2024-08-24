import math
import struct
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import scipy.signal as sps

# Max song length 9 min 21 s (Loading time approx. 6 min 5 s)
flash_bytes = 16777216
sample_rate = 29840
mech_rate = 40

# LEGS, MOUTH, LEFT EYE, RIGHT EYE
mech_toggles = []


def main():
    programming_file = "../Songs/" + input("Programming text file: ")
    in_file = ""

    try:
        with open(programming_file, "r") as f:
            in_file = f.readline().rstrip()

            for i in range(4):
                cur_line = f.readline().rstrip()
                if len(cur_line) > 0:
                    mech_toggles.append(cur_line.split(' '))
                    try:
                        for j in range(len(mech_toggles[i])):
                            mech_toggles[i][j] = float(mech_toggles[i][j])
                    except:
                        print(f"Line {i + 1} includes invalid data!")
                        return
                else:
                    mech_toggles.append([])
    except:
        print("Invalid file!")
        return

    print("Programming file read.")

    sr, data = wavfile.read("../Songs/" + in_file)
    if len(data.shape) > 1:
        datatype = type(data[0][0])
        data = np.average(data, axis=1)
    else:
        datatype = type(data[0])

    # Resample data
    number_of_samples = round(len(data) * float(sample_rate) / sr)
    data = sps.resample(data, number_of_samples).astype(datatype)

    # Convert data to uint8_t
    if np.issubdtype(datatype, np.floating):
        datarange = (-1.0, 1.0)
    else:
        datarange = (np.iinfo((type(data[0]))).min, np.iinfo((type(data[0]))).max)
    data = np.interp(data, datarange, (0, 255))
    data = np.round(data).astype(np.uint8)

    plt.plot(data)
    plt.show()

    t = 0.0
    mech_states = [0, 0, 0, 0]
    mech_is = [0, 0, 0, 0]
    mech_bytes = []
    for i in range(math.ceil(len(data) * (mech_rate / sample_rate) / 2)):
        for j in range(4):
            if len(mech_toggles[j]) > mech_is[j] and t >= mech_toggles[j][mech_is[j]]:
                if mech_states[j] == 0:
                    mech_states[j] = 1
                else:
                    mech_states[j] = 0
                mech_is[j] += 1
        mech_bytes.append((mech_states[3] << 3) | (mech_states[2] << 2) | (mech_states[1] << 1) | mech_states[0])
        t += 1.0 / mech_rate

        for j in range(4):
            if len(mech_toggles[j]) > mech_is[j] and t >= mech_toggles[j][mech_is[j]]:
                if mech_states[j] == 0:
                    mech_states[j] = 1
                else:
                    mech_states[j] = 0
                mech_is[j] += 1
        mech_bytes[-1] |= (mech_states[3] << 7) | (mech_states[2] << 6) | (mech_states[1] << 5) | (mech_states[0] << 4)
        t += 1.0 / mech_rate

    data_bytes = len(data) + len(mech_bytes)
    if 4 + data_bytes > flash_bytes:
        print(f"Too many bytes to write! ({4 + data_bytes}/{flash_bytes})")
    print(f"Writing {4 + data_bytes} bytes to file '{in_file.split('.')[0] + '.ulv'}' ...")

    with open("../Songs/" + in_file.split('.')[0] + '.ulv', "wb") as f:
        f.write(struct.pack("<I", data_bytes))  # Bytes

        mech_i = 0
        for i in range(len(data)):
            if i % ((sample_rate / mech_rate) * 2) == 0:
                f.write(struct.pack("<B", mech_bytes[mech_i]))
                mech_i += 1

            f.write(struct.pack("<B", data[i]))

    print("Done!")
    print()


if __name__ == '__main__':
    main()
