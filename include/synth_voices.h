#include <Arduino.h>

#include "config.h"

struct VoiceProperties {
    unsigned int priority = 0;
    byte channel = 0xff;
    byte pitch = 0xff;

    const bool isAvailable() { return channel == 0xff; }
};

byte findAvailableMusicVoice(int channel, int maxPerChannel);
void setVoiceProperties(byte voice, byte channel, byte pitch);
void resetVoiceProperties(byte voice);
void resetVoiceProperties();
void updateVoiceProperties();
byte findVoice(byte channel);
byte findVoice(byte channel, byte pitch, byte start = 0);

extern VoiceProperties voice_properties[VOICES_COUNT];
