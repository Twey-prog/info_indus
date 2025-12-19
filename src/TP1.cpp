#include "Arduino.h"
#include "lib7seg.h"
#include "pilote_encodeur.h"
#include "stdint.h"
#include <math.h>


// Configuration globale de l'afficheur 7 segments
byte digitPins[] = {2, 3, 4, 5};
byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
Clib7Seg aff7seg(4, digitPins, 8, segmentPins);

// Encodeur OO (KY-040)
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
  aff7seg.refresh();
}

void setup(void) {
  // Initialize serial for debug output
  Serial.begin(9600);
  // Timer setup remains in main
  setupTimer2();
  // Mode button on A4
  pinMode(A4, INPUT_PULLUP);
  // Sign LED on A5
  pinMode(A5, OUTPUT);
  digitalWrite(A5, LOW);
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
      digitalWrite(A5, speed < 0.0f ? HIGH : LOW);
      aff7seg.setNumberFloatAbs(fabs(speed));
    } else {
      digitalWrite(A5, pos < 0 ? HIGH : LOW);
      long v = pos < 0 ? -pos : pos;
      if (v > 9999) {
        // display error EEEE
        aff7seg.setNumberFloatAbs(-1.0f); // library treats negative as error
      } else {
        aff7seg.setNumber((uint32_t)v);
      }
    }
  }
}