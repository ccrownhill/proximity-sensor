#!/usr/bin/python3

__author__ = 'rcope'

from PyHT6022.LibUsbScope import Oscilloscope
import matplotlib.pyplot as plt
import time
import numpy as np
from collections import deque
import sys

if len(sys.argv) != 2:
    sys.stderr.write("Usage: write_data.py <outfile>\n")
    sys.exit(1)
else:
    outfile = sys.argv[1]

sample_rate_index = 24 # Sample rate in MHz
voltage_range = 1
data_points = 3 * 1024

scope = Oscilloscope()
scope.setup()
scope.open_handle()
scope.flash_firmware()
calibration = scope.get_calibration_values() # removes offset
scope.set_interface(1) # choose ISO
scope.set_num_channels(1)
scope.set_sample_rate(sample_rate_index)
scope.set_ch1_voltage_range(voltage_range)
time.sleep(1)

data_points = scope.packetsize # they are the same anyway with 3072

#data = deque(maxlen=2*1024*1024)
data = deque(maxlen=int(24e6))
data_extend = data.extend

def extend_callback(ch1_data, _):
    # skip the first unstable block
    global skip1st
    if skip1st:
        skip1st = False
        return
    
    global data_extend
    data_extend(ch1_data)

skip1st = True

start_time = time.time() + data_points / (sample_rate_index*1e6) # correct the first skipped block
print("Clearing FIFO and starting data transfer...")
i = 0
scope.start_capture()
shutdown_event = scope.read_async(extend_callback, data_points, outstanding_transfers=10, raw=True)

while time.time() - start_time < 1:
    scope.poll()

scope.stop_capture()
print("Stopping new transfers.")
shutdown_event.set()

print("Snooze 1")
time.sleep(1)
print("Closing handle")
scope.close_handle()
print("Handle closed.")

scaled_data = scope.scale_read_data(data, voltage_range)
print("Points in buffer:", len(scaled_data))

with open(outfile, 'w') as file:
    file.write(str(scaled_data)[1:-1].replace(', ', '\n'))

print(f"Data written to {outfile}")
