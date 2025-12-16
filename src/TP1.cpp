#include "Arduino.h"
#include "stdint.h"
#include "lib7seg.h"

void tache1(void);

// Encoder (KY-040) on A0/A1: polled quadrature
volatile long encoderCount = 0;
uint8_t lastEncoded = 0;
const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

// display mode: false=position, true=speed (button on A4)
bool modeSpeed = false;

// Application data
unsigned int datain[4] = {0, 0, 0, 0}; // sampled inputs
uint8_t currentInput = 0;               // selected input index (0..3)
const uint8_t analogPins[4] = {A0, A1, A2, A4}; // wiring: pot4 on A4

void tache2(void);
void tache3(void);

void setupTimer2(){
    cli(); // disable all interrupts
    TCCR2A = (1<<WGM21)|(0<<WGM20); // Mode CTC
    TIMSK2 = (1<<OCIE2A); // Local interruption OCIE2A
    TCCR2B = (0<<WGM22)|(1<<CS22)|(1<<CS21); // prediviser /256
    OCR2A = 250; //250*256*1/16000000 = 4ms
    sei(); // enable all interrupts
    }
    //appelée toutes les 4ms:
  ISR(TIMER2_COMPA_vect){ // timer compare interrupt service routine
  refresh();
}



void setup(void) {
  // Initialize 7-seg hardware (pins, initial states, probe pin)
  setup7seg();
  // Initialize serial for debug output
  Serial.begin(9600);
  // Timer setup remains in main
  setupTimer2();
  // Mode button on A4
  pinMode(A4, INPUT_PULLUP);
  // Encoder pins
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  lastEncoded = (digitalRead(A0) << 1) | digitalRead(A1);
}

void loop() {
    unsigned int periodiciteTache1=100; //100ms entre chaque incrémentation de la valeur à afficher
  static unsigned long timerTache1 = millis();
  static unsigned long timerTache2 = millis();
  static unsigned long timerDisplay = millis();
  const unsigned int displayPeriod = 200; // 5 Hz
  static long lastPos = 0;
  static unsigned long lastPosTime = 0;
  // Task1: sampling every 100ms
  if (millis() - timerTache1 >= periodiciteTache1) {
    timerTache1 += periodiciteTache1;
    tache1();
  }
  // Poll encoder frequently (non-blocking)
  {
    uint8_t MSB = digitalRead(A0);
    uint8_t LSB = digitalRead(A1);
    uint8_t encoded = (MSB << 1) | LSB;
    uint8_t sum = (lastEncoded << 2) | encoded;
    encoderCount += enc_states[sum & 0x0F];
    lastEncoded = encoded;
  }
  // Task2: switch displayed input every 2000ms (2s)
  if (millis() - timerTache2 >= 2000) {
    timerTache2 += 2000;
    tache2();
  }
  // Display update at 5 Hz: show position or speed depending on A4
  if (millis() - timerDisplay >= displayPeriod) {
    timerDisplay += displayPeriod;
    modeSpeed = (digitalRead(A4) == LOW);
    long pos = (long)encoderCount;
    unsigned long now = millis();
    unsigned long dt = now - lastPosTime;
    float speed = 0.0f;
    if (dt > 0) speed = ((float)(pos - lastPos)) * (1000.0f / (float)dt);
    lastPos = pos;
    lastPosTime = now;

    if (modeSpeed) {
      setSign(speed < 0.0f);
      displayFloat(fabs(speed));
    } else {
      setSign(pos < 0);
      long v = pos < 0 ? -pos : pos;
      if (v > 9999) {
        // display error EEEE
        displayFloat(-1.0f); // library treats negative as error
      } else {
        setNumber((uint32_t)v);
      }
    }
  }
  // Task3: keep selected input visible (legacy behaviour)
  tache3();

}
// Task1: sample analog inputs A0..A3 every 100ms
void tache1(void)
{
  for (uint8_t i = 0; i < 4; ++i) {
    datain[i] = (unsigned int)analogRead(analogPins[i]);
  }
  // Print sampled values for observation
  for (int i = 0; i < 4; ++i) {
    Serial.print(datain[i]);
    Serial.print(" ");
  }
  Serial.println();
}

// Task2: every 2s change selected input and set decimal point
void tache2(void)
{
  currentInput = (currentInput + 1) % 4;
  // select dot for current input (bit0 = leftmost)
  uint8_t dots_mask = (uint8_t)(1u << currentInput);
  cli();
  setDots(dots_mask);
  // update displayed number with capped value
  unsigned int v = datain[currentInput];
  if (v > 9999u) v = 9999u;
  setNumber((uint32_t)v);
  sei();
}

// Task3: highest cadence - keep display value in sync with selected input
void tache3(void)
{
  // perform atomic update to avoid ISR reading partially-updated buffer
  cli();
  unsigned int v = datain[currentInput];
  if (v > 9999u) v = 9999u;
  setNumber((uint32_t)v);
  sei();
}