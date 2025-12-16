#ifndef LIB7SEG_H
#define LIB7SEG_H

#include <Arduino.h>
#include <stdint.h>


void setup7seg();

void setDots(uint8_t val);

// Set sign indicator (LED on A5). true => negative (LED on)
void setSign(bool negative);

// Display a positive floating value, choosing decimal scaling to fit 4 digits.
// If value is too large to display, shows EEEE.
void displayFloat(float value);

void refresh();

void setNumber(uint32_t nNumber);

void setDigitHAL(uint8_t nDigitActif, uint8_t nSegsActif);

void setDigit(uint8_t nDigitActif, uint8_t nSegsActif);

#endif // LIB7SEG_H
