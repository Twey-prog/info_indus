#include "Arduino.h"
#include "stdint.h"
#include "lib7seg.h"

void tache1(void);

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
  // Timer setup remains in main
  setupTimer2();
}

void loop() {
    unsigned int periodiciteTache1=100; //100ms entre chaque incrémentation de la valeur à afficher
    static unsigned long timerTache1 = millis();
    if (millis() - timerTache1 >= periodiciteTache1) {
        timerTache1 += periodiciteTache1;
        tache1();
    }

}
void tache1(void)
{
    static uint32_t compteur = 9900;
    static uint8_t dots_local = 0b0001;
    compteur = (compteur + 1) % 10000;
    setNumber(compteur);
    // rotate local dots and push to library
    dots_local = (uint8_t)(((dots_local << 1) | (dots_local >> 3)) & 0x0F);
    setDots(dots_local);
}