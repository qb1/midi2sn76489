#include <Arduino.h>

/*
 * Define internal synths voices control functions.
 */

// Called at setup by synth
void setupSynthVoices();

// Called periodically by synth
void updateSynthVoices();

struct Envelope {
	unsigned int rel;
};

void startVoice(byte voice, byte pitch, Envelope envelope);
void stopVoice(byte voice);
bool isVoiceActive(byte voice);
int voiceVolume(byte voice);
