#include <Arduino.h>

#include "synth.h"
#include "config.h"
#include "synth_oscs.h"
#include "synth_internals.h"
#include "synth_voices.h"
#include "synth_effects.h"
#include "board.h"

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
        startOsc(voice, pitch, channel.envelope);

    case VoiceEffect::Arpeggio:
        startOsc(voice, pitch, channel.envelope);
        effect_properties[voice].arpeggio.pitches[0] = pitch;
        effect_properties[voice].arpeggio.count = 1;
        break;

    default:
        Serial.println("Unhandled effect");
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
        startOsc(voice, pitch, effect.envelope);

    case VoiceEffect::Arpeggio:
    {
        if (effect.arpeggio.count == 0) {
            // It was emptied out, let's start it again
            startOsc(voice, pitch, effect.envelope);
            effect.arpeggio.pitches[0] = pitch;
            effect.arpeggio.count = 1;
        } if (effect.arpeggio.count == ARPEGGIO_MAX_PITCHES) {
            break;
        }else {
            // Sorted add
            for (int i=0; i < ARPEGGIO_MAX_PITCHES; ++i ) {
                if (effect.arpeggio.pitches[i] == 0) {
                    effect.arpeggio.count += 1;
                    effect.arpeggio.pitches[i] = pitch;
                    break;
                }
                if (effect.arpeggio.pitches[i] < pitch) {
                    continue;
                }
                if (effect.arpeggio.pitches[i] == pitch) {
                    // Ensure we don't add the same note twice,
                    // never know what MIDI might throw at us
                    break;
                }
                byte swap = effect.arpeggio.pitches[i];
                effect.arpeggio.pitches[i] = pitch;
                pitch = swap;
            }
            break;
        }
    }

    default:
        Serial.println("Unhandled effect");
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

    case VoiceEffect::Arpeggio:
    {
        int i;
        // Special case for last element
        if (effect.arpeggio.pitches[ARPEGGIO_MAX_PITCHES - 1] == pitch) {
            effect.arpeggio.pitches[ARPEGGIO_MAX_PITCHES - 1] = 0;
            effect.arpeggio.count -= 1;
        } else {
            // Remove without hole
            for (i=0; i < ARPEGGIO_MAX_PITCHES - 1; ++i ) {
                if (effect.arpeggio.pitches[i] == pitch) {
                    effect.arpeggio.count -= 1;
                    break;
                }
            }
            for (; i < ARPEGGIO_MAX_PITCHES - 1 && effect.arpeggio.pitches[i] != 0; ++i) {
                effect.arpeggio.pitches[i] = effect.arpeggio.pitches[i+1];
            }
        }

        if (effect.arpeggio.count == 0) {
            stopOsc(voice);
            effect.time_counter = 0;
            effect.arpeggio.current_pitch_index = 0;
        }
        break;
    }
    }
}

void updateEffects()
{
    for (int voice = 0; voice < VOICES_COUNT; ++voice) {
        auto& effect = effect_properties[voice];

        switch (effect.effect.type) {
        case VoiceEffect::None:
            break;

        case VoiceEffect::Arpeggio:
        {
            if (effect.arpeggio.count == 0) {
                break;
            }

            effect.time_counter += REFRESH_RATE;
            if (effect.time_counter > effect.effect.speed / effect.arpeggio.count) {
                effect.time_counter = 0;

                effect.arpeggio.current_pitch_index += 1;
                effect.arpeggio.current_pitch_index %= effect.arpeggio.count;
                startOsc(voice, effect.arpeggio.pitches[effect.arpeggio.current_pitch_index], effect.envelope);
            }
            break;
        }
        }
    }
}
