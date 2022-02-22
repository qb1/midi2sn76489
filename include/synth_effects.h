#pragma once
#include <Arduino.h>

#include "config.h"

struct EffectProperties {
    VoiceEffect effect;
    Envelope envelope;

    int time_counter = 0;

    // For arpeggio effect
    struct {
        byte pitches[ARPEGGIO_MAX_PITCHES] = { 0 };
        byte current_pitch_index = 0;
        byte count = 0;
    } arpeggio;
};

// Called at setup by synth
void setupEffects();

// Called periodically by synth
void updateEffects();

void setEffect(byte voice, const SynthChannel& channel, byte pitch, byte velocity);
void updateEffectProperties(byte voice, const SynthChannel& channel);
void updateEffectAdd(byte voice, byte pitch, byte velocity);
void updateEffectRemove(byte voice, byte pitch);

extern EffectProperties effect_properties[VOICES_COUNT];
