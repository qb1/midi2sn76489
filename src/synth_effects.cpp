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

    switch (effect_properties[voice].effect.type)
    {
    case VoiceEffect::None:
        startOsc(voice, pitch, velocity, channel.envelope);

    default:
        ERROR_MSG("Unhandled effect ", effect_properties[voice].effect.type);
        break;
    }
}

void updateEffectProperties(byte voice, const SynthChannel& channel)
{
    effect_properties[voice].effect = channel.effect;
    effect_properties[voice].envelope = channel.envelope;
}

void updateEffectAdd(byte voice, byte pitch, byte velocity)
{
    auto& effect = effect_properties[voice];

    switch (effect.effect.type)
    {
    case VoiceEffect::None:
        startOsc(voice, pitch, velocity, effect.envelope);

    default:
        ERROR_MSG("Unhandled effect ", effect.effect.type);
        break;
    }
}

void updateEffectRemove(byte voice, byte pitch)
{
    auto& effect = effect_properties[voice];
    switch (effect.effect.type)
    {
    case VoiceEffect::None:
        if (voice_properties[voice].pitch == pitch) {
            stopOsc(voice);
        }
        break;
    }
}

void updateEffects()
{
    for (int voice = 0; voice < VOICES_COUNT; ++voice) {
        auto& effect = effect_properties[voice];

        switch (effect.effect.type) {
        case VoiceEffect::None:
            break;
        }
    }
}
