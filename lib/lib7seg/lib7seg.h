#ifndef LIB7SEG_H
#define LIB7SEG_H

#include <Arduino.h>
#include <stdint.h>

/**
 * @class Clib7Seg
 * @brief Class for controlling a multiplexed 7-segment display
 * 
 * This class manages a multi-digit 7-segment display with common cathode configuration.
 * It supports dynamic allocation of pin configurations and provides methods for
 * displaying numbers, floating-point values, and controlling decimal points.
 */
class Clib7Seg
{
private:
    uint8_t numDigits;      ///< Number of digits in the display
    uint8_t numSegments;    ///< Number of segments per digit (typically 8 including DP)
    uint8_t *digitPins;     ///< Array of digit control pins (dynamically allocated)
    uint8_t *segmentPins;   ///< Array of segment control pins (dynamically allocated)
    uint8_t *digits;        ///< Array storing digit values 0-15 (dynamically allocated)
    uint8_t dots;           ///< Bitmask for decimal points (1 bit per digit)
    
    /**
     * @brief Initialize hardware pins and internal state
     */
    void init();
    
    /**
     * @brief Set a specific digit with segment pattern using digitalWrite (HAL method)
     * @param nDigitActif Index of the digit to activate (0-based)
     * @param nSegsActif Segment pattern byte (bit 0-6 = segments a-g, bit 7 = DP)
     */
    void setDigitHAL(uint8_t nDigitActif, uint8_t nSegsActif);

public:
    /**
     * @brief Default constructor using predefined pins
     * 
     * Uses default pin configuration:
     * - Digits: pins 2, 3, 4, 5
     * - Segments: pins 6, 7, 8, 9, 10, 11, 12, 13
     */
    Clib7Seg();
    
    /**
     * @brief Parameterized constructor with custom pin configuration
     * @param numDigitsInit Number of digits
     * @param digitPinsInit Array of digit control pins
     * @param numSegmentsInit Number of segments (typically 8)
     * @param segmentPinsInit Array of segment control pins
     * 
     * Memory is dynamically allocated using new[] for all pin and digit arrays.
     */
    Clib7Seg(uint8_t numDigitsInit, uint8_t *digitPinsInit, 
             uint8_t numSegmentsInit, uint8_t *segmentPinsInit);
    
    /**
     * @brief Destructor - frees dynamically allocated memory
     */
    ~Clib7Seg();
    
    /**
     * @brief Set display to show an unsigned integer value
     * @param val Value to display (0-9999 for 4-digit display)
     * 
     * Values exceeding display capacity are clamped to maximum (e.g., 9999).
     */
    void setNumber(uint32_t val);
    
    /**
     * @brief Set decimal point bitmask
     * @param val Bitmask where bit 0 = leftmost digit DP, bit 1 = second digit, etc.
     */
    void setDots(uint8_t val);
    
    /**
     * @brief Display a positive floating-point value with auto-scaling
     * @param floatAbs Absolute (positive) floating value to display
     * 
     * Automatically selects decimal point position to fit value in available digits.
     * Displays "EEEE" if value is negative, NaN, or too large to represent.
     */
    void setNumberFloatAbs(float floatAbs);
    
    /**
     * @brief Refresh one digit of the display (multiplexing)
     * 
     * Should be called periodically (e.g., every 4ms) from a timer ISR
     * to maintain display brightness and avoid ghosting.
     */
    void refresh();
};

#endif // LIB7SEG_H
