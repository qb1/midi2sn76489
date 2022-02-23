#pragma once
#include <Arduino.h>

/*
 * Define internal synths voices control functions.
 */
struct Envelope;

// Called at setup by synth
void setupSynthOscs();

// Called periodically by synth
void updateSynthOscs();

// Start (or restart) an oscillator with an envelope
void startOsc(byte osc, byte pitch, byte velocity, const Envelope& envelope, bool pitch_is_noise_control = false);

// Change oscillator's pitch, do not start or restart it. Will not alter current envelope.
void moveOsc(byte osc, byte pitch, bool pitch_is_noise_control = false);

// Move oscillator to release - does not stop the oscillator right away
void stopOsc(byte osc);

// Is osc in use
bool isOscActive(byte osc);

// Is osc being released - prime candidate for recycling
bool isOscReleasing(byte osc);

// Returns volume of osc
// Note: if being modulated, does not return current volume but rather
// target volume - avoids considering attacking note as off
int  oscTargetVolume(byte osc);
