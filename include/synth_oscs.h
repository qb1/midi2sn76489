#include <Arduino.h>

/*
 * Define internal synths voices control functions.
 */
struct Envelope;

// Called at setup by synth
void setupSynthOscs();

// Called periodically by synth
void updateSynthOscs();

void startOsc(byte osc, byte pitch, const Envelope& envelope);
void stopOsc(byte osc);
bool isOscActive(byte osc);
int oscVolume(byte osc);
