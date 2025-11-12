#include "LineSensor.h"

// --- TUNING PARAMETERS ---
// Your ADC is 12-bit (0-4095). A dark surface (the line)
// reflects less light, so the sensor voltage is HIGH.
// You will need to tune this value.
#define LINE_THRESHOLD 3000

// For Demo 1, "mostly-dark" means carpet.
// If more than ~85% of pixels are "dark", we are on the carpet.
// (128 pixels * 0.85) approx 110. Tune this as needed.
#define CARPET_PIXEL_COUNT 110
// --- END TUNING ---

/**
 * @brief Calculates line position using a weighted average.
 * This will give a "slow and wobbly" result, perfect for Demo 1.
 */
int32_t LineSensor_Calculate_Error(uint16_t* sensorValues) {
    long weighted_sum = 0; // Stores (value * position)
    long pixel_sum = 0;    // Stores sum of all sensor values
    int pixels_on_line = 0;
    
    for (int i = 0; i < 128; i++) {
        uint16_t val = sensorValues[i];
        
        // Only include pixels that are "dark enough"
        if (val > LINE_THRESHOLD) {
            // This is a weighted average.
            // A pixel at index 0 is -64 from center.
            // A pixel at index 127 is +63 from center.
            // We use (i - 64) as the "weight" for the position.
            weighted_sum += (long)val * (i - 64);
            pixel_sum += val;
            pixels_on_line++;
        }
    }

    // If no pixels see the line, we're lost. Go straight.
    if (pixels_on_line == 0 || pixel_sum == 0) {
        return 0; 
    }

    // The result is the weighted average of the error.
    // e.g., if the line is on the right, weighted_sum will be positive.
    // if on the left, it will be negative.
    return weighted_sum / pixel_sum;
}

bool LineSensor_Detect_Carpet(uint16_t* sensorValues) {
    int dark_pixel_count = 0;
    for (int i = 0; i < 128; i++) {
        if (sensorValues[i] > LINE_THRESHOLD) {
            dark_pixel_count++;
        }
    }
    
    // Return true if we meet the "mostly-dark" requirement
    return (dark_pixel_count > CARPET_PIXEL_COUNT);
}
