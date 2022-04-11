#include <Arduino.h>

#include "synth.h"
#include "config.h"
#include "synth_oscs.h"
#include "synth_internals.h"
#include "synth_voices.h"
#include "synth_effects.h"
#include "board.h"

// #define ENABLE_DEBUG_LOGS
#include "logs.h"

void updateEffects()
{
    static int16_t amounts[16] = {0};
    for (int channel = 0; channel < 16; ++channel) {
        auto& synth_channel = synthChannels[channel];
        if (synth_channel.isNone()) {
            continue;
        }

        auto& modulation = synth_channel.effect.modulation;
        if (modulation.amount == 0) {
            continue;
        }

        modulation.position += REFRESH_RATE;
        modulation.position %= modulation.speed;
                
        auto position = modulation.position % (modulation.speed / 2);
        auto half_pos = modulation.position % (modulation.speed / 4);
        int16_t amount = half_pos * 100 / (modulation.speed / 4);
        if (position >= modulation.speed / 4) {
            amount = 100 - amount;
        }
        if (modulation.position >= modulation.speed / 2) {
            amount = -amount;
        }

        amounts[channel] = amount * modulation.amount / 100;
    }

    for (int voice = 0; voice < VOICES_COUNT; ++voice) {
        if (!isOscActive(voice)) {
            continue;
        }

        auto channel = voice_properties[voice].channel;
        if (channel > 16) {
            ERROR_MSG("Active voice with non-sensical channel ", channel)
        }

        auto& synth_channel = synthChannels[channel];

        if (synth_channel.effect.portamento.position != 0xffff) {
            // Portamento 
            synth_channel.effect.portamento.position += REFRESH_RATE;
            if (synth_channel.effect.portamento.position > synth_channel.effect.portamento.speed) {
                synth_channel.effect.portamento.position = -1;
                moveOsc(voice, voice_properties[voice].pitch);
            } else {
                int16_t from_pitch = synth_channel.effect.portamento.from_pitch;
                int16_t to_pitch = voice_properties[voice].pitch;
                int16_t total_bend = to_pitch - from_pitch;
                int16_t current_bend = 100 * (uint32_t)synth_channel.effect.portamento.position / (uint32_t)synth_channel.effect.portamento.speed;
                DEBUG_MSG("Portamento bending ", from_pitch, ":", to_pitch, " @", current_bend, "%");
                bendOsc(voice, current_bend, total_bend);
            }
        } else if (synth_channel.effect.modulation.amount != 0 &&
                   synth_channel.effect.modulation.type == VoiceEffect::Modulation::Vibrato) {
            // Vibrato and portamento are exclusive
            bendOsc(voice, amounts[channel]);
        }

        if (synth_channel.effect.modulation.amount != 0 &&
                   synth_channel.effect.modulation.type == VoiceEffect::Modulation::Tremolo) {
            // Tremolo
            tremoloOsc(voice, amounts[channel]);
        }
    }
}
