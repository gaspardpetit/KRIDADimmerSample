//-----------------------------------------------
// Modified sample code for KRIDA Electronics dimmer
// https://www.facebook.com/krida.electronics/
//
// - Modified to switch from 50Hz to 60Hz more easily
// - Modified to use an interrupt instead of delay
//-----------------------------------------------

// change the pins if necessary - make sure
// the interrupt pin supports interrupts
enum {
  PIN_ZERO_INTERRUPT = 2, // Pin used for zero-crossing interrupt
  PIN_AC_LOAD = 3         // Output to Opto Triac pin
};

// change to 50 if you are on 50Hz AC
static const int AC_FREQUENCY = 60;

volatile unsigned char dimming = 3;  // Dimming level (0-100)


class TIMER1
{
  public:
    typedef void (*Callback)();
    static Callback m_callback;

    static void disable()
    {
      noInterrupts();
      TCCR1A = 0;
      TCCR1B = 0;
      TCNT1 = 0;
      TIMSK1 &= ~(1 << OCIE1A); // disable timer compare interrupt
      interrupts();
    }

    static void reset(unsigned int microseconds, Callback cb)
    {
      noInterrupts();

      m_callback = cb;

      TCCR1A = 0;
      TCCR1B = 0;
      TCNT1 = 0;

      OCR1A = microseconds * 2 + 1; // compare match register 16MHz/8/(1000000/microseconds)
      TCCR1B |= (1 << WGM12); // CTC mode
      TCCR1B |= (1 << CS11); // 256 prescaler
      TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

      interrupts();
    }

    static void onInterrupt()
    {
      if (m_callback != NULL)
      {
        m_callback();
        disable();
      }
    }
};

TIMER1::Callback TIMER1::m_callback;

ISR(TIMER1_COMPA_vect) // timer compare interrupt service routine
{
  TIMER1::onInterrupt();
}

unsigned long DELAY_START = 0;
unsigned long DELAY_TIME = 0;
long DELAY_TIME_MIN = -1;
long DELAY_TIME_MAX = -1;

void OnCross0V();

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_AC_LOAD, OUTPUT);// Set AC Load pin as output
  attachInterrupt(digitalPinToInterrupt(PIN_ZERO_INTERRUPT), OnCross0V, RISING);

  Serial.begin(9600);
}

static const unsigned long PERIOD_MICRO = 1000000 / AC_FREQUENCY;
static const unsigned long HALF_PERIOD_MICRO = PERIOD_MICRO / 2;
void UpdateDelayTime()
{
  noInterrupts();
  DELAY_TIME = micros() - DELAY_START;
  if (DELAY_TIME_MIN < 0)
    DELAY_TIME_MIN = DELAY_TIME;
  else
    DELAY_TIME_MIN = min(DELAY_TIME, DELAY_TIME_MAX);

  if (DELAY_TIME_MAX < 0)
    DELAY_TIME_MAX = DELAY_TIME;
  else
    DELAY_TIME_MAX = max(DELAY_TIME, DELAY_TIME_MIN);
    interrupts();
}

void DelayAndOpen(int dimtime)
{
  delayMicroseconds(dimtime);
  // send impulse to turn on immediately.
  UpdateDelayTime();
  digitalWrite(PIN_AC_LOAD, HIGH);
  delayMicroseconds(10);
  // this will not immediately turn off the dimmer, it will be
  // automatically turned off at the next zero-crossing
  digitalWrite(PIN_AC_LOAD, LOW);
}

void (*DELAY_AND_OPEN_STRATEGY)(int dimtime) = DelayAndOpenAsync;

void DelayAndOpenAsync(int dimtime)
{
  if (HALF_PERIOD_MICRO - dimtime < 300)
  {
    // we have less than 300 microseconds, do not take
    // the risk to turn on in the next half cycle, keep
    // 100% off.
    TIMER1::reset(0, NULL);
  }
  else
  {
    // if we open too quickly, we sometimes end up opening
    // before the dimmer automatically turns off.
    if (dimtime < 100)
      dimtime = 100;

    TIMER1::reset(dimtime, [] {
      // send impulse to turn on immediately.
      UpdateDelayTime();

      digitalWrite(PIN_AC_LOAD, HIGH);
      delayMicroseconds(10);
      // this will not immediately turn off the dimmer, it will be
      // automatically turned off at the next zero-crossing
      digitalWrite(PIN_AC_LOAD, LOW);
    });
  }
}

void OnCross0V()  // function to be fired at the zero crossing to dim the light
{
  int dimtime = (HALF_PERIOD_MICRO / 100 * dimming);
  DELAY_START = micros();
  DELAY_AND_OPEN_STRATEGY(dimtime);
}


void printResults(unsigned long counter)
{
    noInterrupts();
    unsigned long delayTime = DELAY_TIME;
    unsigned long delayMin = DELAY_TIME_MIN;
    unsigned long delayMax = DELAY_TIME_MAX;
    DELAY_TIME_MIN = -1;
    DELAY_TIME_MAX = -1;
    interrupts();

    Serial.print("with dimmer at ");
    Serial.print(dimming);
    Serial.print(", can count to ");
    Serial.print(counter);
    Serial.print(" in 1 second [error:");
    unsigned long accuracy = delayMax - delayMin;
    Serial.print(accuracy);
    Serial.print(" min:");
    Serial.print(delayMin);
    Serial.print(" max:");
    Serial.print(delayMax);
    Serial.println("]");
}


void loop() {

  DELAY_AND_OPEN_STRATEGY = DelayAndOpen;
  Serial.println("USING BLOCKING STRATEGY");

  for (int i = 10; i <= 90; i += 10)
  {
    dimming = i;
    unsigned long start = micros();
    unsigned long counter = 0;
    while (micros() - start < 1000000)
      ++counter;

    printResults(counter);
  }

  DELAY_AND_OPEN_STRATEGY = DelayAndOpenAsync;
  Serial.println("USING ASYNC STRATEGY");

  for (int i = 0; i <= 100; i += 10)
  {
    dimming = i;
    unsigned long start = micros();
    unsigned long counter = 0;
    while (micros() - start < 1000000)
      ++counter;

    printResults(counter);
  }
}

