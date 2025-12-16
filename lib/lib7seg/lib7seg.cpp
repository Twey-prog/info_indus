#include "lib7seg.h"
#include <math.h>

// Table de décodage 7 segments (cathode commune)
static const uint8_t tabDeco7seg[] = {0x3f, 0x06, 0x5b, 0x4F, 0x66, 0x6d, 0x7d, 0x07,
                                       0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

// Broches par défaut pour le constructeur sans paramètre
static const uint8_t defaultDigitPins[] = {2, 3, 4, 5};
static const uint8_t defaultSegmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};

Clib7Seg::Clib7Seg()
{
    // Constructeur par défaut : utilise les broches par défaut
    numDigits = sizeof(defaultDigitPins) / sizeof(defaultDigitPins[0]);
    numSegments = sizeof(defaultSegmentPins) / sizeof(defaultSegmentPins[0]);
    
    // Allocation dynamique
    digitPins = new uint8_t[numDigits];
    for (uint8_t i = 0; i < numDigits; i++) {
        digitPins[i] = defaultDigitPins[i];
    }
    
    segmentPins = new uint8_t[numSegments];
    for (uint8_t i = 0; i < numSegments; i++) {
        segmentPins[i] = defaultSegmentPins[i];
    }
    
    digits = new uint8_t[numDigits];
    
    init();
}

Clib7Seg::Clib7Seg(uint8_t numDigitsInit, uint8_t *digitPinsInit, uint8_t numSegmentsInit, uint8_t *segmentPinsInit)
{
    numDigits = numDigitsInit;
    numSegments = numSegmentsInit;
    
    // Allocation dynamique et copie des broches
    digitPins = new uint8_t[numDigits];
    for (uint8_t i = 0; i < numDigits; i++) {
        digitPins[i] = digitPinsInit[i];
    }
    
    segmentPins = new uint8_t[numSegments];
    for (uint8_t i = 0; i < numSegments; i++) {
        segmentPins[i] = segmentPinsInit[i];
    }
    
    digits = new uint8_t[numDigits];
    
    init();
}



Clib7Seg::~Clib7Seg()
{
    // Libération de la mémoire allouée
    delete[] digitPins;
    delete[] segmentPins;
    delete[] digits;
}

void Clib7Seg::init()
{
    // Configure les broches des digits en sortie
    for (uint8_t i = 0; i < numDigits; i++) {
        pinMode(digitPins[i], OUTPUT);
        digitalWrite(digitPins[i], HIGH); // désactivé (actif bas)
    }
    // Configure les broches des segments en sortie
    for (uint8_t j = 0; j < numSegments; j++) {
        pinMode(segmentPins[j], OUTPUT);
        digitalWrite(segmentPins[j], LOW);
    }
    // Initialise le masque de points et les chiffres
    dots = 0;
    for (uint8_t i = 0; i < numDigits; i++) {
        digits[i] = 0;
    }
}

void Clib7Seg::setNumber(uint32_t val)
{
    if (val > 9999U) val = 9999U;
    uint32_t n = val;
    for (int i = 0; i < (int)numDigits; i++) {
        digits[numDigits - 1 - i] = n % 10;
        n /= 10;
    }
}

void Clib7Seg::setDots(uint8_t val)
{
    dots = val & 0xFF;
}

void Clib7Seg::setNumberFloatAbs(float floatAbs)
{
    // IMPLEMENTATION POUR 4 DIGITS SEULEMENT
    if (!(floatAbs >= 0.0f)) {
        // Affiche EEEE pour valeur invalide
        for (uint8_t i = 0; i < numDigits && i < 4; i++) {
            digits[i] = 14; // E dans la table
        }
        return;
    }
    
    // Trouve le nombre de décimales pour tenir sur 4 digits
    int chosen_d = -1;
    uint32_t intval = 0;
    for (int d = 3; d >= 0; --d) {
        double scaled = floatAbs * pow(10.0, d);
        uint32_t cand = (uint32_t)round(scaled);
        if (cand < 10000u) {
            chosen_d = d;
            intval = cand;
            break;
        }
    }
    
    if (chosen_d < 0) {
        // Trop grand : affiche EEEE
        for (uint8_t i = 0; i < numDigits && i < 4; i++) {
            digits[i] = 14;
        }
        return;
    }
    
    // Remplit les digits
    for (int i = 0; i < (int)numDigits && i < 4; i++) {
        digits[numDigits - 1 - i] = intval % 10;
        intval /= 10;
    }
    
    // Positionne le point
    if (chosen_d > 0) {
        int dotIndex = numDigits - 1 - chosen_d;
        if (dotIndex >= 0 && dotIndex < (int)numDigits) {
            dots = (1u << dotIndex);
        } else {
            dots = 0;
        }
    } else {
        dots = 0;
    }
}

void Clib7Seg::refresh()
{
    static uint8_t digitActif = 0;
    // Construit le masque de segments pour le digit actif
    uint8_t raw = digits[digitActif];
    uint8_t segs = tabDeco7seg[raw & 0x0F];
    // Ajoute le point si nécessaire
    if ((dots >> digitActif) & 1) {
        segs |= 0x80;
    }
    // Affiche le digit
    setDigitHAL(digitActif, segs);
    digitActif = (digitActif + 1) % numDigits;
}

void Clib7Seg::setDigitHAL(uint8_t nDigitActif, uint8_t nSegsActif)
{
    // Désactive tous les digits (actif bas)
    for (uint8_t i = 0; i < numDigits; i++) {
        digitalWrite(digitPins[i], HIGH);
    }
    // Configure les segments
    for (uint8_t i = 0; i < numSegments; i++) {
        digitalWrite(segmentPins[i], (nSegsActif >> i) & 0x01);
    }
    // Active le digit sélectionné (actif bas)
    if (nDigitActif < numDigits) {
        digitalWrite(digitPins[nDigitActif], LOW);
    }
}
