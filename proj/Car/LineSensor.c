#include "LineSensor.h"

// --- TUNING PARAMETERS ---
// We no longer need a fixed LINE_THRESHOLD.

// This is a new "sensitivity" check. If the difference between the
// darkest and lightest pixel is less than this, we'll assume we're
// on a uniform surface (all carpet or all track) and are "lost".
#define MINIMUM_LINE_RANGE 1000

// We still use this to decide if we're on carpet, but it will
// now be compared against the *dynamic* threshold.
#define CARPET_PIXEL_COUNT 500
// --- END TUNING ---

/**
 * @brief Calculates line position using a dynamic threshold and weighted average.
 */
int32_t LineSensor_Calculate_Error(uint16_t* sensorValues) {
    long weighted_sum = 0; // Stores (value * position)
    long pixel_sum = 0;    // Stores sum of all sensor values
    int pixels_on_line = 0;
    
    // --- 1. Find Min/Max and Calculate Dynamic Threshold ---
    uint16_t min_val = 4095;
    uint16_t max_val = 0;
    
    for (int i = 0; i < 128; i++) {
        if (sensorValues[i] < min_val) min_val = sensorValues[i];
        if (sensorValues[i] > max_val) max_val = sensorValues[i];
    }

    // Check if we even see a line (is there enough contrast?)
    if ((max_val - min_val) < MINIMUM_LINE_RANGE) {
        return 0; // We are "lost" or on a uniform surface, go straight.
    }

    // Calculate the dynamic threshold
    uint16_t dynamic_threshold = (min_val + max_val) / 2;

    // --- 2. Calculate Weighted Average using the new threshold ---
    for (int i = 0; i < 128; i++) {
        uint16_t val = sensorValues[i];
        
        // Use the new dynamic threshold
        if (val > dynamic_threshold) {
            // (i - 64) gives a position error from the center
            weighted_sum += (long)val * (i - 64);
            pixel_sum += val;
            pixels_on_line++;
        }
    }

    if (pixels_on_line == 0 || pixel_sum == 0) {
        return 0; // Lost line
    }

    return weighted_sum / pixel_sum;
}

/**
 * @brief Checks if the camera sees "mostly-dark" carpet.
 * This now also uses the dynamic threshold.
 */
bool LineSensor_Detect_Carpet(uint16_t* sensorValues) {
    int dark_pixel_count = 0;

    // --- 1. Find Min/Max and Calculate Dynamic Threshold ---
    uint16_t min_val = 4095;
    uint16_t max_val = 0;
    
    for (int i = 0; i < 128; i++) {
        if (sensorValues[i] < min_val) min_val = sensorValues[i];
        if (sensorValues[i] > max_val) max_val = sensorValues[i];
    }
    
    uint16_t dynamic_threshold = (min_val + max_val) / 2;

    // --- 2. Count "dark" pixels based on the dynamic threshold ---
    for (int i = 0; i < 128; i++) {
        if (sensorValues[i] > dynamic_threshold) {
            dark_pixel_count++;
        }
    }
    
    // Return true if we meet the "mostly-dark" requirement
    return (dark_pixel_count > CARPET_PIXEL_COUNT);
}
