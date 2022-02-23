#include <Arduino.h>

#include "synth.h"
#include "config.h"
#include "synth_oscs.h"
#include "synth_internals.h"
#include "synth_voices.h"
#include "synth_effects.h"
#include "board.h"

//#define ENABLE_DEBUG_LOGS
#include "logs.h"

void setupSynth() {
    setupEffects();
    setupVoiceProperties();
    setupSynthOscs();
}

void updateSynth() {
    updateSynthOscs();
    updateEffects();
}

void stopNoteOnChannel(byte midiChannel, byte pitch)
{
    byte voice = findVoice(midiChannel, pitch);
    while (voice != 0xff) {
        stopOsc(voice);
        voice = findVoice(midiChannel, pitch, voice + 1);
    }
}

void startNoteDrum(const SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findAvailableVoice(synth_channel.midiChannel, synth_channel.voiceCount, SynthChannel::Drum);
    if (voice == 0xff) {
        Serial.println("No available drum voice");
        return;
    }
    setVoiceProperties(voice, synth_channel.midiChannel, pitch);
    startOsc(voice, pitch, velocity, synth_channel.envelope, true /* pitch is noise value */);
}

void startNotePolyphonic(const SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findAvailableVoice(synth_channel.midiChannel, synth_channel.voiceCount, SynthChannel::Music);
    if (voice == 0xff) {
        Serial.println("No available music voice");
        return;
    }
    setVoiceProperties(voice, synth_channel.midiChannel, pitch);
    startOsc(voice, pitch, velocity, synth_channel.envelope);
}

void startNoteSingle(const SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findVoice(synth_channel.midiChannel);
    if (voice == 0xff) {
        byte voice = findAvailableVoice(synth_channel.midiChannel, synth_channel.voiceCount, SynthChannel::Music);
        if (voice == 0xff) {
            Serial.println("No available voice");
            return;
        }

        setVoiceProperties(voice, synth_channel.midiChannel, pitch);
        setEffect(voice, synth_channel, pitch, velocity);
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

    if (synth_channel.type == SynthChannel::Drum) {
        startNoteDrum(synth_channel, pitch, velocity);
    } else if (synth_channel.voiceCount > 1) {
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

void synthConfUpdated(const SynthChannel& channel)
{
    for (int i=0; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel == channel.midiChannel) {
            updateEffectProperties(i, channel);
        }
    }
}

void controlChange(byte channel, byte control, byte value) {

	DEBUG_MSG("Control change: control=", control, ", value=", value, ", channel=", channel);

    switch (control) {
    case 121:
    case 123:
        // Panic or reset
        INFO_MSG("MIDI Reset.");
        muteAll();
        setupSynth();
        break;
    }

    if (synthChannels[channel] == nullptr) {
        return;
    }
    auto& synth_channel = *synthChannels[channel];

    switch (control) {
    case 1: // Modulation (right one ? probably not.)
        synth_channel.envelope.rel = 5000 / 127 * value;
        synthConfUpdated(synth_channel);
        break;
    case 2: // Portamento time
        synth_channel.effect.speed = 500 / 127 * value;
        synthConfUpdated(synth_channel);
        break;
    }
}
