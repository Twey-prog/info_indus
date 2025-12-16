#include "lib7seg.h"
#include <math.h>

// Hardware configuration and state
uint8_t numDigits = 4;
uint8_t digitPins[] = {2, 3, 4, 5};
uint8_t numSegments = 8;
uint8_t segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
uint8_t tabDeco7seg[] = {0x3f, 0x06, 0x5b, 0x4F, 0x66, 0x6d, 0x7d, 0x07,
                         0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

// Display state
uint8_t number[4] = {0, 0, 0, 0};    // encoded segment bytes for each digit
static uint8_t rawDigits[4] = {0,0,0,0}; // numeric digits 0..9
static uint8_t dots = 0b0001;

void setup7seg()
{
    for (uint8_t i = 0; i < sizeof(digitPins) / sizeof(digitPins[0]); i++) {
        pinMode(digitPins[i], OUTPUT);
        digitalWrite(digitPins[i], LOW);
    }
    for (uint8_t j = 0; j < numSegments; j++) {
        pinMode(segmentPins[j], OUTPUT);
        digitalWrite(segmentPins[j], LOW);
    }

    DDRC |= (1 << 3);   // PC3 as output (probe)
    PORTC &= ~(1 << 3); // PC3 low

    // Configure sign LED on A5
    pinMode(A5, OUTPUT);
    digitalWrite(A5, LOW);
}

void setSign(bool negative)
{
    digitalWrite(A5, negative ? HIGH : LOW);
}

void displayFloat(float value)
{
    if (!(value >= 0.0f)) {
        // For negative or NaN, display EEEE
        for (int i = 0; i < 4; ++i) rawDigits[i] = 14; // 'E'
        for (int i = 0; i < 4; ++i) number[i] = tabDeco7seg[rawDigits[i]];
        return;
    }

    // Choose decimal places 0..3 so value*10^d < 10000
    int chosen_d = -1;
    uint32_t intval = 0;
    for (int d = 3; d >= 0; --d) {
        double scaled = value * pow(10.0, d);
        uint32_t cand = (uint32_t)round(scaled);
        if (cand < 10000u) {
            chosen_d = d;
            intval = cand;
            break;
        }
    }

    if (chosen_d < 0) {
        // value too large -> show EEEE
        for (int i = 0; i < 4; ++i) rawDigits[i] = 14; // 'E'
        for (int i = 0; i < 4; ++i) number[i] = tabDeco7seg[rawDigits[i]];
        return;
    }

    // Fill rawDigits with intval (MSB at index 0)
    for (int i = 0; i < 4; ++i) {
        rawDigits[3 - i] = intval % 10;
        intval /= 10;
    }

    // Build dots mask: place decimal point after numDigits - 1 - chosen_d
    uint8_t dots_mask = 0;
    if (chosen_d > 0) {
        int dotIndex = numDigits - 1 - chosen_d;
        if (dotIndex >= 0 && dotIndex < numDigits) dots_mask = (1u << dotIndex);
    }

    dots = dots_mask & 0x0F;
    for (int i = 0; i < 4; i++) {
        number[i] = tabDeco7seg[rawDigits[i]] | ((dots >> i) & 1 ? 0x80 : 0x00);
    }
}

void setDots(uint8_t val)
{
    dots = val & 0x0F;
    for (int i = 0; i < 4; ++i) {
        number[i] = tabDeco7seg[rawDigits[i]] | ((dots >> i) & 1 ? 0x80 : 0x00);
    }
}

void setNumber(uint32_t nNumber)
{
    if (nNumber > 9999U) nNumber = 9999U;
    for (int i = 0; i < 4; i++) {
        rawDigits[3 - i] = nNumber % 10;
        nNumber /= 10;
    }
    for (int i = 0; i < 4; i++) {
        number[i] = tabDeco7seg[rawDigits[i]] | ((dots >> i) & 1 ? 0x80 : 0x00);
    }
}

void refresh()
{
    static uint8_t digitActif = 0;
    setDigit(digitActif, number[digitActif]);
    digitActif = (digitActif + 1) % numDigits;
}

void setDigitHAL(uint8_t nDigitActif, uint8_t nSegsActif)
{
  // disable all digits
  for (uint8_t i = 0; i < numDigits; i++) {
    digitalWrite(digitPins[i], HIGH);
  }
  // set segments
  for (uint8_t i = 0; i < numSegments; i++) {
      digitalWrite(segmentPins[i], (nSegsActif >> i) & 0x01);
  }
  // enable the selected digit (active low)
  digitalWrite(digitPins[nDigitActif], LOW);
}

void setDigit(uint8_t nDigitActif, uint8_t nSegsActif)
{
  // TIC: set PC3 high
  PORTC |= (1 << 3);

  // deactivate all digits (PD2..PD5) -> set them high (active low)
  const uint8_t DIGIT_MASK = (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
  PORTD |= DIGIT_MASK;

  // low-weight segment bits -> PD6, PD7 (bits 0,1)
  uint8_t pd_seg_bits = (((nSegsActif >> 0) & 0x01) << 6) | (((nSegsActif >> 1) & 0x01) << 7);
  PORTD = (PORTD & ~((1 << 6) | (1 << 7))) | pd_seg_bits;

  // PB0..PB1 <- bits 2..3
  uint8_t pb_low_mask = 0x03; // PB0..PB1
  uint8_t pb_low_bits = (uint8_t)((nSegsActif >> 2) & 0x03);
  PORTB = (PORTB & ~pb_low_mask) | pb_low_bits;

  // PB2..PB5 <- bits 4..7
  uint8_t pb_high_mask = 0x3C; // PB2..PB5
  uint8_t pb_high_bits = (uint8_t)(((nSegsActif >> 4) & 0x0F) << 2);
  PORTB = (PORTB & ~pb_high_mask) | pb_high_bits;

  // activate the requested digit (drive low PD2..PD5)
  if (nDigitActif < numDigits) {
    PORTD &= ~(1 << (nDigitActif + 2));
  }

  // TOC: set PC3 low
  PORTC &= ~(1 << 3);
}
