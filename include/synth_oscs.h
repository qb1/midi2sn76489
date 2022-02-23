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
bool isOscActive(byte osc);
int oscVolume(byte osc);
