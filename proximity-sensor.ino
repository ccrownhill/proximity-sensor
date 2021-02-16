/***********************************
To execute this connect the output of an oscillator to pin 5.
This script will measure the frequency of the square wave (from 5V to 0V) signal
input to pin 5 and trigger a change in frequency.

Problem: With pulseIn only frequencies of up to 50kHz can be measured
***********************************/

//const int input = 11;
//double pulseTime, frequency;
//
//void setup() {
//  Serial.begin(9600);
//  Serial.println("Proximity Sensor");
//}
//
//void loop() {
//  pulseTime = pulseIn(input, HIGH);
//  Serial.print("Pulsetime: ");
//  Serial.println(pulseTime);
//
//  frequency = 1.E6 / (2 * pulseTime);
//
//  Serial.println(frequency);
//  delay(1000);
//}




// Timer and Counter example
// Author: Nick Gammon
// Date: 17th January 2012

// Input: Pin D5

// these are checked for in the main program
volatile unsigned long timerCounts;
volatile boolean counterReady;

// internal to counting routine
unsigned long overflowCount;
unsigned int timerTicks;
unsigned int timerPeriod;

void startCounting (unsigned int ms)
{
  counterReady = false;         // time not up yet
  timerPeriod = ms;             // how many 1 ms counts to do
  timerTicks = 0;               // reset interrupt counter
  overflowCount = 0;            // no overflows yet

  // reset Timer 1 and Timer 2
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;
  TCCR2B = 0;

  // Timer 1 - counts events on pin D5
  TIMSK1 = bit (TOIE1);   // interrupt on Timer 1 overflow

  // Timer 2 - gives us our 1 ms counting interval
  // 16 MHz clock (62.5 ns per tick) - prescaled by 128
  //  counter increments every 8 µs.
  // So we count 125 of them, giving exactly 1000 µs (1 ms)
  TCCR2A = bit (WGM21) ;   // CTC mode
  OCR2A  = 124;            // count up to 125  (zero relative!!!!)

  // Timer 2 - interrupt on match (ie. every 1 ms)
  TIMSK2 = bit (OCIE2A);   // enable Timer2 Interrupt

  TCNT1 = 0;      // Both counters to zero
  TCNT2 = 0;

  // Reset prescalers
  GTCCR = bit (PSRASY);        // reset prescaler now
  // start Timer 2
  TCCR2B =  bit (CS20) | bit (CS22) ;  // prescaler of 128
  // start Timer 1
  // External clock source on T1 pin (D5). Clock on rising edge.
  TCCR1B =  bit (CS10) | bit (CS11) | bit (CS12);
}  // end of startCounting

ISR (TIMER1_OVF_vect)
{
  ++overflowCount;               // count number of Counter1 overflows
}  // end of TIMER1_OVF_vect


//******************************************************************
//  Timer2 Interrupt Service is invoked by hardware Timer 2 every 1 ms = 1000 Hz
//  16Mhz / 128 / 125 = 1000 Hz

ISR (TIMER2_COMPA_vect)
{
  // grab counter value before it changes any more
  unsigned int timer1CounterValue;
  timer1CounterValue = TCNT1;  // see datasheet, page 117 (accessing 16-bit registers)
  unsigned long overflowCopy = overflowCount;

  // see if we have reached timing period
  if (++timerTicks < timerPeriod)
    return;  // not yet

  // if just missed an overflow
  if ((TIFR1 & bit (TOV1)) && timer1CounterValue < 256)
    overflowCopy++;

  // end of gate time, measurement ready

  TCCR1A = 0;    // stop timer 1
  TCCR1B = 0;

  TCCR2A = 0;    // stop timer 2
  TCCR2B = 0;

  TIMSK1 = 0;    // disable Timer1 Interrupt
  TIMSK2 = 0;    // disable Timer2 Interrupt

  // calculate total count
  timerCounts = (overflowCopy << 16) + timer1CounterValue;  // each overflow is 65536 more
  counterReady = true;              // set global flag for end count period
}  // end of TIMER2_COMPA_vect


const int button = 2;
int currentState;

const int led_pins[3] = { 8, 12, 13 }; 

// this will be set upon button press
float init_frq = 0;
float delta_frq = 0;

const int sensitivity = 1000;
const int led_step_multiplier = 5;

void setup ()
{
  Serial.begin(115200);
  Serial.println("Proximity Sensor");

  pinMode(button, INPUT_PULLUP);

  for (byte i = 0; i < 3; i++) {
    pinMode(led_pins[i], OUTPUT);
    digitalWrite(led_pins[i], LOW);
  }
} // end of setup

void loop ()
{
  // stop Timer 0 interrupts from throwing the count out
  byte oldTCCR0A = TCCR0A;
  byte oldTCCR0B = TCCR0B;
  TCCR0A = 0;    // stop timer 0
  TCCR0B = 0;

  startCounting (500);  // how many ms to count for

  while (!counterReady)
     { }  // loop until count over

  // adjust counts by counting interval to give frequency in Hz
  float frq = (timerCounts *  1000.0) / timerPeriod;

  check_button(frq);

//  Serial.print ("Frequency: ");
//  Serial.print ((unsigned long) frq);
//  Serial.println (" Hz.");

  if (init_frq != 0) {
    delta_frq = frq - init_frq;
    delta_frq = abs(delta_frq);

    if (delta_frq > sensitivity) {
      Serial.println("Triggered LED 1");
      digitalWrite(led_pins[0], HIGH);
    }

    if (delta_frq > sensitivity + sensitivity * led_step_multiplier) {
      Serial.println("Triggered LED 2");
      digitalWrite(led_pins[1], HIGH);
    }

    if (delta_frq > sensitivity + sensitivity * led_step_multiplier * 2) {
      Serial.println("Triggered LED 3");
      digitalWrite(led_pins[2], HIGH);
    }
  }

  // restart timer 0
  TCCR0A = oldTCCR0A;
  TCCR0B = oldTCCR0B;

  // let serial stuff finish
  delay(200);
  
  switch_off_led();
}   // end of loop

void check_button (float current_frequency)
{
  currentState = digitalRead(button);

  if (currentState == LOW) {
    init_frq = current_frequency;

    Serial.print("Init frequency set to ");
    Serial.println(current_frequency);
  }
}

void switch_off_led ()
{
  for (byte i = 0; i < 3; i++) {
    digitalWrite(led_pins[i], LOW);
  }
}
