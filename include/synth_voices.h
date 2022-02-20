#include <Arduino.h>

/*
 * Define internal synths voices control functions.
 */

// Called at setup by synth
void setupSynthVoices();

// Called periodically by synth
void updateSynthVoices();

void startVoice(byte voice, byte pitch);
void stopVoice(byte voice);
bool isVoiceActive(byte voice);
