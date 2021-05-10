# Proximity sensor

This is a project which uses a Colpitt's Oscillator (a certain LC circuit) to detect metallic objects in the vicinity of its coil by registering changes in the frequency of oscillation of the LC circuit. These changes are detected and indicated using LEDs by an arduino.

## `proximity-sensor.ino`

To find out if a frequency changes you first need to be able to measure it. For this I want to point to these ways to do that:

* [One Transistor Frequency Counter](https://www.onetransistor.eu/2018/09/how-to-count-frequency-with-arduino.html)
* [Nick Gammon Frequency Counter](http://www.gammon.com.au/timers)

`proximity-sensor.ino` uses the one from One Transistor and then builds up logic on top of the frequency counting to trigger changes in frequency.
