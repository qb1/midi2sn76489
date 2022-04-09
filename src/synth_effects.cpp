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

        auto& vibrato = synth_channel.effect.vibrato;
        if (vibrato.amount == 0) {
            continue;
        }

        vibrato.position += REFRESH_RATE;
        vibrato.position %= vibrato.speed;
                
        auto position = vibrato.position % (vibrato.speed / 2);
        auto half_pos = vibrato.position % (vibrato.speed / 4);
        int16_t amount = half_pos * 100 / (vibrato.speed / 4);
        if (position >= vibrato.speed / 4) {
            amount = 100 - amount;
        }
        if (vibrato.position >= vibrato.speed / 2) {
            amount = -amount;
        }

        amounts[channel] = amount * vibrato.amount / 100;
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

        if (synth_channel.effect.vibrato.amount != 0) {
            bendOsc(voice, amounts[channel]);
        }
    }
}
