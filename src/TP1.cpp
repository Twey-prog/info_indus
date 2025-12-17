#include "Arduino.h"
#include "stdint.h"

uint8_t numDigits = 4;
uint8_t digitPins[] = {2, 3, 4, 5};
uint8_t numSegments = 8;
uint8_t segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
uint8_t tabDeco7seg[]={0x3f,0x06,0x5b,0x4F,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
uint8_t number[4] = {0, 0, 0, 0};
uint8_t dots = 0b0001; 


void tache1(void);
void tache2(void);
void setNumber(uint32_t nNumber);
void setDigit(uint8_t nDigitActif, uint8_t nSegsActif);

void setup(void) {
    for (uint8_t i = 0; i < sizeof(digitPins) / sizeof(digitPins[0]); i++) {
        pinMode(digitPins[i], OUTPUT);
        digitalWrite(digitPins[i], LOW);
    }
    for (uint8_t j = 0; j < numSegments; j++) {
        pinMode(segmentPins[j], OUTPUT);
        digitalWrite(segmentPins[j], LOW);
    }
}

void loop() {
    unsigned int periodiciteTache1=100; //100ms entre chaque incrémentation de la valeur à afficher
    unsigned int periodiciteTache2=4; //4ms pour l'affichage de chaque digit
    static unsigned long timerTache1 = millis();
    static unsigned long timerTache2 = millis();
    if (millis() - timerTache1 >= periodiciteTache1) {
        timerTache1 += periodiciteTache1;
        tache1();
    }
    if (millis() - timerTache2 >= periodiciteTache2) {
        timerTache2 += periodiciteTache2;
        tache2();
    }
}

void tache1(void)
{
    static uint32_t compteur = 0;
    compteur = (compteur + 1) % 10000;
    setNumber(compteur);
    dots = ((dots << 1) | (dots >> 3)) & 0b00001111; // faire tourner les points
}

void tache2(void)
{
    static uint8_t digitActif = 0;
    setDigit(digitActif, number[digitActif]);
    digitActif = (digitActif + 1) % numDigits;
}

void setNumber(uint32_t nNumber)
{
  for (int i = 0; i < 4; i++)
  { 
    number[3 - i] = nNumber % 10; 
    nNumber /= 10;                
  }
  for (int i = 0; i < 4; i++)
  {
    number[i] = tabDeco7seg[number[i]] + ((dots >> i) & 1 ? 0b10000000 : 0x00);
  }
}





void setDigit(uint8_t nDigitActif, uint8_t nSegsActif)
{
  // désactiver tous les digits avant de changer les segments
  for (uint8_t i = 0; i < numDigits; i++)
  {
    digitalWrite(digitPins[i], HIGH);
  }
  // piloter les segments en utilisant les valeurs des bits de nSegsActif
  for (uint8_t i = 0; i < numSegments; i++)
  {
      digitalWrite(segmentPins[i], (nSegsActif >> i) & 0x01);

  }
  // activer le bon digit
  digitalWrite(digitPins[nDigitActif], LOW);
}