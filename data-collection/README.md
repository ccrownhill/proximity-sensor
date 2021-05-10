## Data collection for my proximity sensor project

All data was collected with an Hantek6022BE and the [OpenHantek API](https://github.com/Ho-Ro/Hantek6022API).

This is how data was aquired:

* `write_data.py` will put all values measured from the Oscilloscope during 1 second into a file. `write_data_series.py` is similar but will write a set of data from whole measurement series to files in a specified directory.

* `freq_and_volt_series_eval.py` will evaluate the all data files in specified directory. This evaluated data can then be used for investigations and data plotting.

## Shape measurements

All shape measurements were conducted at a distance of **0.64 cm** from the coil.

## Distance measurements

All distance measurements were conducted using a **3x3 cm copper sheet**.

One check frequency before every copper sheet (2x data)
