#pragma once
#include <Arduino.h>

#include "config.h"
#include "synth_internals.h"

struct VoiceProperties {
    uint32_t priority = 0;
    byte channel = 0xff;
    byte pitch = 0xff;

    bool isAvailable() const { return channel == 0xff; }
};

// Called at setup by synth
void setupVoiceProperties();

byte findAvailableVoice(int channel, int onChip, int maxPerChannel, SynthChannel::Type type);
void setVoiceProperties(byte voice, byte channel, byte pitch);
void resetVoiceProperties(byte voice);
void updateVoiceProperties();
byte findVoice(byte channel);
byte findVoice(byte channel, byte pitch, byte start = 0);

extern VoiceProperties voice_properties[VOICES_COUNT];
