#include <Arduino.h>

#include "synth.h"
#include "config.h"
#include "synth_oscs.h"
#include "synth_internals.h"
#include "synth_voices.h"
#include "board.h"

struct EffectProperties {
    VoiceEffect effect;

    int time_counter = 0;

    // For arpeggio effect
    struct {
        byte pitches[ARPEGGIO_MAX_PITCHES] = { 0 };
        byte current_pitch_index = 0;
        byte count = 0;
    } arpeggio;
};

void updateVoiceEffects();

EffectProperties effect_properties[VOICES_COUNT];

Envelope envelope;

void setupSynth() {
    setupSynthOscs();
    resetVoiceProperties();
    envelope = { .rel = 1000, };
}

void updateSynth() {
    updateSynthOscs();
    updateVoiceEffects();
}

void stopNoteOnChannel(byte midiChannel, byte pitch)
{
    byte voice = findVoice(midiChannel, pitch);
    while (voice != 0xff) {
        Serial.print("stop voice");
        Serial.println(voice);
        stopOsc(voice);
        voice = findVoice(midiChannel, pitch, voice + 1);
    }
}

void setEffect(byte voice, const VoiceEffect& effect, byte pitch, byte velocity)
{
    effect_properties[voice] = EffectProperties();

    effect_properties[voice].effect = effect;

    switch (effect.type)
    {
    case VoiceEffect::None:
        startOsc(voice, pitch, envelope);

    case VoiceEffect::Arpeggio:
        startOsc(voice, pitch, envelope);
        effect_properties[voice].arpeggio.pitches[0] = pitch;
        effect_properties[voice].arpeggio.count = 1;
        break;

    default:
        Serial.println("Unhandled effect");
        break;
    }
}

void updateEffectAdd(byte voice, byte pitch, byte velocity)
{
    auto& effect = effect_properties[voice];

    switch (effect.effect.type)
    {
    case VoiceEffect::None:
        startOsc(voice, pitch, envelope);

    case VoiceEffect::Arpeggio:
    {
        if (effect.arpeggio.count == 0) {
            // It was emptied out, let's start it again
            startOsc(voice, pitch, envelope);
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

void updateVoiceEffects()
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
                startOsc(voice, effect.arpeggio.pitches[effect.arpeggio.current_pitch_index], envelope);
            }
            break;
        }
        }
    }
}

void startNotePolyphonic(const SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findAvailableMusicVoice(synth_channel.midiChannel, synth_channel.voiceCount);
    if (voice == 0xff) {
        Serial.println("No available voice");
        return;
    }
    setVoiceProperties(voice, synth_channel.midiChannel, pitch);
    startOsc(voice, pitch, envelope);
}

void startNoteSingle(const SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findVoice(synth_channel.midiChannel);
    if (voice == 0xff) {
        byte voice = findAvailableMusicVoice(synth_channel.midiChannel, synth_channel.voiceCount);
        if (voice == 0xff) {
            Serial.println("No available voice");
            return;
        }

        setVoiceProperties(voice, synth_channel.midiChannel, pitch);
        setEffect(voice, synth_channel.effect, pitch, velocity);
    } else {
        setVoiceProperties(voice, synth_channel.midiChannel, pitch);
        updateEffectAdd(voice, pitch, velocity);
    }
}

void noteOn(byte channel, byte pitch, byte velocity) {
	/*Serial.print("noteOn channel=");
	Serial.print(channel);
	Serial.print(", pitch=");
	Serial.println(pitch);*/

    if (synthChannels[channel] == nullptr) {
        return;
    }
    auto& synth_channel = *synthChannels[channel];

    if (synth_channel.voiceCount > 1) {
        startNotePolyphonic(synth_channel, pitch, velocity);
    } else {
        startNoteSingle(synth_channel, pitch, velocity);
    }
}

void noteOff(byte channel, byte pitch, byte velocity) {
    if (synthChannels[channel] == nullptr) {
        return;
    }
    auto& synth_channel = *synthChannels[channel];

    if (synth_channel.voiceCount > 1) {
        stopNoteOnChannel(synth_channel.midiChannel, pitch);
    } else {
        byte voice = findVoice(synth_channel.midiChannel);
        if (voice != 0xff) {
            updateEffectRemove(voice, pitch);
        }
    }
}

void controlChange(byte channel, byte control, byte value) {
	Serial.print("Control change: control=");
	Serial.print(control);
	Serial.print(", value=");
	Serial.print(value);
	Serial.print(", channel=");
	Serial.println(channel);


    if (control == 123 or control == 121) {
        // Panic or reset
        muteAll();
        setupSynth();
    }
    envelope.rel = 5000 / 127 * value;
}
