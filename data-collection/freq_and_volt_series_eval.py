#!/usr/bin/python3

import pandas as pd
import numpy as np
import math
import sys
import os
from scipy.signal import find_peaks

if len(sys.argv) != 3:
    sys.stderr.write(f"{sys.argv[0]}: <data_dir> <out_dir>\n")
    sys.exit(1)
else:
    data_dir = sys.argv[1]
    out_dir = sys.argv[2]

# constants
multiplier = 10 # because probe is set to 10x
sample_rate = 24e6 # sample rate in Hz
sample_time = 1 # time of measuring in s

# outfiles
peak2peakfile = open(out_dir + '/peak2peak', 'w')
frequencyfile = open(out_dir + '/frequency', 'w')

# sort volt files according to the number at the end of their name
dir_list = sorted(os.listdir(data_dir), key=lambda item: int(item[5:]))
dir_list = [data_dir + "/" + item for item in dir_list]


for voltfile in dir_list:
    print(f"Starting analysis of '{voltfile}' ...")

    voltages = pd.read_csv(voltfile, header=None).to_numpy().reshape((-1,))
    num_samples = voltages.shape[0]
    time_between_samples = 1/sample_rate
    calculated_sample_time = num_samples * time_between_samples

    max_indexes = find_peaks(voltages, height=0.2, distance=15)[0]
    max_freq = len(max_indexes) / calculated_sample_time
    max_mean = np.mean(voltages[max_indexes])

    min_indexes = find_peaks(voltages*(-1), height=0.2, distance=15)[0]
    min_freq = len(min_indexes) / calculated_sample_time
    min_mean = np.mean(voltages[min_indexes])

    avg_freq = (max_freq + min_freq) / 2
    vpp = round((max_mean - min_mean) * multiplier, 3)

    peak2peakfile.write(str(vpp) + '\n')
    frequencyfile.write(str(avg_freq) + '\n')

    peak2peakfile.flush()
    frequencyfile.flush()

    print(f"Analysis data written to '{out_dir}'.\n")


peak2peakfile.close()
frequencyfile.close()
