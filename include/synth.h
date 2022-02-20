#include <Arduino.h>

/*
 * Define synths control functions.
 */

// Must be called at setup
void setupSynth();

// Must be call periodically on REFRESH_RATE
void updateSynth();

void noteOn(byte channel, byte pitch, byte velocity);
void noteOff(byte channel, byte pitch, byte velocity);
void controlChange(byte channel, byte control, byte value);
