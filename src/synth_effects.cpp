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

EffectProperties effect_properties[VOICES_COUNT];

void setupEffects()
{
    for (int i=0; i < VOICES_COUNT; ++i) {
        effect_properties[i] = EffectProperties();
    }
}

void setEffect(byte voice, const SynthChannel& channel, byte pitch, byte velocity)
{
    effect_properties[voice] = EffectProperties();

    effect_properties[voice].effect = channel.effect;
    effect_properties[voice].envelope = channel.envelope;

    // Restart effects temporality
    effect_properties[voice].effect.vibrato.position = 0;
}

void updateEffectProperties(byte voice, const SynthChannel& channel)
{
    auto tmp = effect_properties[voice].effect;
    effect_properties[voice].effect = channel.effect;
    effect_properties[voice].envelope = channel.envelope;

    // Update should not affect temporality of effects
    effect_properties[voice].effect.vibrato.position = tmp.vibrato.position;
}

void updateEffectAdd(byte voice, byte pitch, byte velocity)
{
    auto& effect = effect_properties[voice];

    // Restart effects temporality
    effect.effect.vibrato.position = 0;
}

void updateEffectRemove(byte voice, byte pitch)
{
}

void updateEffects()
{
    /*for (int voice = 0; voice < VOICES_COUNT; ++voice) {
        if (!isOscActive(voice)) {
            return;
        }

        auto& effect = effect_properties[voice];

        if (effect.effect.vibrato.amount != 0) {
            effect.effect.vibrato.position += REFRESH_RATE;
            if (effect.effect.vibrato.position % 5 == 0) {
                effect.effect.vibrato.position %= effect.effect.vibrato.speed * 2;
                
                auto position = effect.effect.vibrato.position % effect.effect.vibrato.speed;
                auto half_pos = effect.effect.vibrato.position % (effect.effect.vibrato.speed / 2);
                int8_t amount = half_pos * 100 / (effect.effect.vibrato.speed / 2);
                if (position > effect.effect.vibrato.speed / 2) {
                    amount = 100 - amount;
                }
                if (effect.effect.vibrato.position >= effect.effect.vibrato.speed) {
                    amount = -amount;
                }

                bendOsc(voice, amount);
            }
        }
    }*/
}
