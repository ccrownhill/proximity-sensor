/*
 * The code here comes largely from https://github.com/onetransistor/arduino-projects/blob/master/freq_count/freq_count.ino
 * which is a frequency counter program for the arduino (from One Transistor, 2018, https://www.onetransistor.eu).
 * This code here extends this program to trigger a certain change in frequency
 * as indicated by three LEDs (the bigger the change, the more LEDs will light up)
 *
 * Input pin: 5 (signal of frequency up to 6 MHz)
 * Button to set default frequency (the frequency all others will be compared to) upon press: 7
 * Pins of LEDs to show if there is a change: 8, 9, 10
 */

// button to set the reference frequency to the currently measured frequency
const int button = 7;
int currentState;
int lastState = HIGH;

// connect LEDs to these pins
const int led_pins[3] = {8, 9, 10}; 

// this will be set upon button press
float init_frq = 0;
float delta_frq = 0;

const int sensitivity = 1000;
const int led_step_multiplier = 5;

// set sampling period here (in milliseconds):
unsigned int samplingPeriod = 200;

// Timer 1 overflows counter
volatile unsigned long overflow1;

volatile float freqHz;

void init_Timer1() {
  overflow1 = 0; // reset overflow counter

  // Set control registers (see datasheet)
  TCCR1A = 0; // normal mode of operation
  TCCR1B = bit(CS12) | bit(CS11) | bit(CS10); // use external clock source

  TCNT1 = 0; // set current timer value to 0

  TIMSK1 = bit(TOIE1); // enable interrupt on overflow
}

ISR(TIMER1_OVF_vect) {
  overflow1++; // increment overflow counter
}

// Timer 2 overflows counter
volatile unsigned int overflow2;

void init_Timer2() {
  overflow2 = 0; // reset overflow counter

  GTCCR = bit(PSRASY); // reset prescalers

  // Set control registers (see datasheet)
  TCCR2A = bit(WGM21); // CTC mode
  TCCR2B = bit(CS22) | bit(CS20); // prescaler set to 1/128, "ticks" at 125 kHz
  OCR2A = 124; // counts from 0 to 124, then fire interrupt and reset;

  TCNT2 = 0; // set current timer value to 0

  TIMSK2 = bit(OCIE2A); // enable interrupt
}

// interrupt happens at each 125 counts / 125 kHz = 0.001 seconds = 1 ms
ISR(TIMER2_COMPA_vect) {
  if (++overflow2 < samplingPeriod) // add an overflow and check if it's ready
    return; // still sampling

  unsigned long totalSamples = (overflow1 << 16) + TCNT1;  
    
  /*float */freqHz = totalSamples * 1000.0 / samplingPeriod;

  Serial.print("Frequency: ");
  Serial.print((unsigned long)freqHz);
  Serial.println("Hz");


  if (init_frq != 0) {
    delta_frq = freqHz - init_frq;
    delta_frq = abs(delta_frq);

    if (delta_frq > sensitivity) {
      Serial.println("Triggered LED 1");
      digitalWrite(led_pins[0], HIGH);

      if (delta_frq > sensitivity + sensitivity * led_step_multiplier) {
        Serial.println("Triggered LED 2");
        digitalWrite(led_pins[1], HIGH);

        if (delta_frq > sensitivity + sensitivity * led_step_multiplier * led_step_multiplier) {
          Serial.println("Triggered LED 3");
          digitalWrite(led_pins[2], HIGH);
        } else {
          digitalWrite(led_pins[2], LOW);
        }
      } else {
        digitalWrite(led_pins[2], LOW);
        digitalWrite(led_pins[1], LOW);
      }
    } else {
      digitalWrite(led_pins[2], LOW);
      digitalWrite(led_pins[1], LOW);
      digitalWrite(led_pins[0], LOW);
    }
  }

  // reset timers
  TCNT1 = 0; overflow1 = 0;
  TCNT2 = 0; overflow2 = 0;
}



void check_button (float current_frequency) {
  currentState = digitalRead(button);

  if (currentState == LOW && lastState == HIGH) {
    init_frq = current_frequency;

    Serial.print("Init frequency set to ");
    Serial.println(current_frequency);
  }

  lastState = currentState;
}

void switch_off_led () {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(led_pins[i], LOW);
  }
}



void setup() {
  // enable serial output
  Serial.begin(115200);
  Serial.println("Arduino Frequency Counter");
  Serial.println();

  pinMode(button, INPUT_PULLUP);

  for (byte i = 0; i < 3; i++) {
    pinMode(led_pins[i], OUTPUT);
    digitalWrite(led_pins[i], LOW);
  }
  
  // Disable Timer0; millis() will no longer work
  TCCR0A = 0; TCCR0B = 0;

  // start timer 1 (count frequency)
  init_Timer1();
  init_Timer2();
}

void loop() {
  // nothing here; interrupts perform everything
  // you can add user input that changes sampling period
  check_button(freqHz);
}
