#pragma once
#include <Arduino.h>

/*
 * Define internal synths voices control functions.
 */
struct Envelope;
struct SynthChannel;

// Called at setup by synth
void setupSynthOscs();

// Called periodically by synth
void updateSynthOscs();

// Start (or restart) an oscillator with an envelope
void startOsc(byte osc, byte pitch, byte velocity, const SynthChannel& channel, const Envelope& envelope, bool pitch_is_noise_control = false);

// Change oscillator's pitch, do not start or restart it. Will keep current envelope, unless if Release
void moveOsc(byte osc, byte pitch, bool pitch_is_noise_control = false);

// Bend current oscillator's pitch, expressed in signed percent between 0 and 2 semi-tones
// Do not use on noise channels
void bendOsc(byte osc, int bend, int semitones = 2);

// Signal an external channel volume change on this osc
void signalOscVolumeChange(byte osc);

// Signal an external channel volume change on this osc
void tremoloOsc(byte osc, int amount);

// Move oscillator to release - does not stop the oscillator right away
void stopOsc(byte osc);

// Is osc in use
bool isOscActive(byte osc);

// Is osc being released - prime candidate for recycling
bool isOscReleasing(byte osc);

// Is osc in use
bool isOscLegatoReady(byte osc);

// Returns volume of osc
// Note: if being modulated, does not return current volume but rather
// target volume - avoids considering attacking note as off
int  oscTargetVolume(byte osc);
