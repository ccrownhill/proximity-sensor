/***********************************
To execute this connect the output of an oscillator to pin 11.
This script will measure the frequency of the square wave (from 5V to 0V) signal
input to pin 11 and trigger a change in frequency.
***********************************/

const int input = 11;
double pulseTime, frequency;

void setup() {
  Serial.begin(9600);
  pinMode(11, INPUT);
}

void loop() {
  pulseTime = pulseIn(input, HIGH, 5000);
  frequency = 1.E6 / (2 * pulseTime);

  Serial.println(frequency);
  delay(1000);
}
