//-----------------------------------------------
// Modified sample code for KRIDA Electronics dimmer
// https://www.facebook.com/krida.electronics/
//
// - Modified to switch from 50Hz to 60Hz more easily
//-----------------------------------------------

// change the pins if necessary - make sure
// the interrupt pin supports interrupts
enum {
  PIN_ZERO_INTERRUPT = 3, // Pin used for zero-crossing interrupt
  PIN_AC_LOAD = 7         // Output to Opto Triac pin
};

// change to 50 if you are on 50Hz AC
static const int AC_FREQUENCY = 60;

volatile unsigned char dimming = 3;  // Dimming level (0-100)

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_AC_LOAD, OUTPUT);// Set AC Load pin as output
  attachInterrupt(digitalPinToInterrupt(PIN_ZERO_INTERRUPT), zero_crosss_int, RISING);

  // Serial.begin(9600);
}

void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  static const unsigned long PERIOD_MICRO = 1000000 / AC_FREQUENCY;
  static const unsigned long HALF_PERIOD_MICRO = PERIOD_MICRO / 2;

  int dimtime = (HALF_PERIOD_MICRO / 100 * dimming);

  // the longer we wait before firing, the lower the output voltage
  delayMicroseconds(dimtime);

  // send impulse to turn on immediately.
  digitalWrite(PIN_AC_LOAD, HIGH);
  delayMicroseconds(10);
  // this will not immediately turn off the dimmer, it will be
  // automatically turned off at the next zero-crossing
  digitalWrite(PIN_AC_LOAD, LOW);
}



void loop() {
  for (int i = 5; i < 85; i++)
  {
    dimming = i;
    delay(20);
  }

  for (int i = 85; i > 5; i--)
  {
    dimming = i;
    delay(20);
  }
}

