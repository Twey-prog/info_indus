#include "Arduino.h"
#include "lib7seg.h"
#include "pilote_encodeur.h"
#include "stdint.h"
#include <math.h>

// Encoder (KY-040) using OO library
CStateMachineRotaryWithSpeed encoder(A0, A1);

// display mode: false=position, true=speed (button on A4)
bool modeSpeed = false;

void setupTimer2() {
  cli();                                // disable all interrupts
  TCCR2A = (1 << WGM21) | (0 << WGM20); // Mode CTC
  TIMSK2 = (1 << OCIE2A);               // Local interruption OCIE2A
  TCCR2B = (0 << WGM22) | (1 << CS22) | (1 << CS21); // prediviser /256
  OCR2A = 250;                                       // 250*256*1/16000000 = 4ms
  sei();                                             // enable all interrupts
}
// appelée toutes les 4ms:
ISR(TIMER2_COMPA_vect) { // timer compare interrupt service routine
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
}

void loop() {
  static unsigned long timerDisplay = millis();
  const unsigned int displayPeriod = 200; // 5 Hz

  // Poll encoder as fast as possible for correct quadrature tracking
  encoder.clock();

  // Display update at 5 Hz: show position or speed depending on A4
  if (millis() - timerDisplay >= displayPeriod) {
    timerDisplay += displayPeriod;
    modeSpeed = (digitalRead(A4) == LOW);

    long pos = encoder.getPosition();
    float speed = encoder.getSpeed();

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
}