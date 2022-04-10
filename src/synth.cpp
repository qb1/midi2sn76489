#include <Arduino.h>

#include "synth.h"
#include "config.h"
#include "synth_oscs.h"
#include "synth_internals.h"
#include "synth_voices.h"
#include "synth_effects.h"
#include "board.h"

#define ENABLE_DEBUG_LOGS
#include "logs.h"

void setupSynth() {
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
    byte voice = findAvailableVoice(synth_channel.midiChannel, synth_channel.onChip, synth_channel.voiceCount, SynthChannel::Drum);
    if (voice == 0xff) {
        Serial.println("No available drum voice");
        return;
    }
    setVoiceProperties(voice, synth_channel.midiChannel, pitch);
    startOsc(voice, pitch, velocity, synth_channel, drumDefinitionFromPitch(pitch).envelope, true /* is noise */);
}

void startNotePolyphonic(const SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findAvailableVoice(synth_channel.midiChannel, synth_channel.onChip, synth_channel.voiceCount, SynthChannel::Music);
    if (voice == 0xff) {
        Serial.println("No available music voice");
        return;
    }

    setVoiceProperties(voice, synth_channel.midiChannel, pitch);
    // No legato possible on polyphonic mode
    startOsc(voice, pitch, velocity, synth_channel, synth_channel.envelope);
    // We should probably start the osc pre-bended,
    // but I can't hear any artifact in doing after the fact and the code is simpler so...
    bendOsc(voice, synth_channel.effect.current_bend);
}

void startNoteSingle(SynthChannel& synth_channel, byte pitch, byte velocity)
{
    byte voice = findVoice(synth_channel.midiChannel);
    if (voice == 0xff) {
        voice = findAvailableVoice(synth_channel.midiChannel, synth_channel.onChip, synth_channel.voiceCount, SynthChannel::Music);
        if (voice == 0xff) {
            Serial.println("No available music voice");
            return;
        }
    }

    auto previous_pitch = voice_properties[voice].pitch;
    setVoiceProperties(voice, synth_channel.midiChannel, pitch);
    if (synth_channel.effect.legato && isOscLegatoReady(voice)) {
        if (synth_channel.effect.portamento.speed == 0) {
            moveOsc(voice, pitch);
        } else {            
            moveOsc(voice, previous_pitch); // A glissando might be on-going, abort it.
            synth_channel.effect.portamento.from_pitch = previous_pitch;
            synth_channel.effect.portamento.position = 0; // Kickstart glissando effect
        }
    } else {
        startOsc(voice, pitch, velocity, synth_channel, synth_channel.envelope);
    }
    bendOsc(voice, synth_channel.effect.current_bend);
}

void noteOn(byte channel, byte pitch, byte velocity) {
	/*Serial.print("noteOn channel=");
	Serial.print(channel);
	Serial.print(", pitch=");
	Serial.println(pitch);*/
    if (synthChannels[channel].isNone()) {
        return;
    }
    auto& synth_channel = synthChannels[channel];

    if (synth_channel.type == SynthChannel::Drum) {
        startNoteDrum(synth_channel, pitch, velocity);
    } else if (synth_channel.voiceCount > 1) {
        startNotePolyphonic(synth_channel, pitch, velocity);
    } else {
        startNoteSingle(synth_channel, pitch, velocity);
    }
}

void noteOff(byte channel, byte pitch, byte velocity) {
    if (synthChannels[channel].isNone()) {
        return;
    }
    auto& synth_channel = synthChannels[channel];

    if (synth_channel.voiceCount > 1) {
        stopNoteOnChannel(synth_channel.midiChannel, pitch);
    } else {
        byte voice = findVoice(synth_channel.midiChannel);
        if (voice != 0xff) {
            stopNoteOnChannel(synth_channel.midiChannel, pitch);
            //updateEffectRemove(voice, pitch);
        }
    }
}

void controlChange(byte channel, byte control, byte bvalue)
{
    unsigned long value = bvalue;
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

    if (synthChannels[channel].isNone()) {
        return;
    }
    auto& synth_channel = synthChannels[channel];

    switch (control) {

    case 0: // Bank Select?
        bendChange(channel, value); // Wtf bitwig transforms my keyboard bend into CC#0 when passthrough.
        break;

    case 7: // Channel volume
        volumeChange(channel, value);
        break;

    case 1: // Modulation = vibrato or tremolo
        synth_channel.effect.modulation.amount = value;
        break;
    case 12: // Effect control one: for us, select vibrato (off) or tremolo (on)
             // This is an arbitrary choice: I can't find a more appropriate CC value.
        synth_channel.effect.modulation.type = (value < 64 ? VoiceEffect::Modulation::Vibrato 
                                                           : VoiceEffect::Modulation::Tremolo);
        break;
    case 76: // Vibrato rate (or tremolo rate if effect is selected)
        synth_channel.effect.modulation.speed = 5 * 127 - (5 * value); // From 635 to 0 ms
        break;

    case 72: // Release time
        synth_channel.envelope.rel = 25 * value; // From 0 to 3175 ms
        break;
    case 73: // Attack time
        synth_channel.envelope.attack = 5 * value; // From 0 to 635 ms
        break;
    case 75: // Decay time
        synth_channel.envelope.decay = 5 * value; // From 0 to 635 ms
        break;
    case 64: // Sustain pedal will drive sustain volume (not meant for that use but hey)
        synth_channel.envelope.sustain = 15 * value / 127;
        break;

    case 68: // Legato footswitch: OFF if < 64
        synth_channel.effect.legato = (value >= 64);
        break;        

    case 5: // Portamento time
        synth_channel.effect.portamento.speed = 15 * value;
        break;

    // Used when experimenting with drums
    /*case 7:
        INFO_MSG("Setting drum white noise to ", (bool)value);
        value = value ? 0b111 : 0b011;
        for (int i=0; i < 12; ++i) {
            drumDefinitions[i].noise = value;
        }
        break;*/
    }
}

void bendChange(byte channel, byte bvalue)
{
    if (synthChannels[channel].isNone()) {
        return;
    }
    auto& synth_channel = synthChannels[channel];

    int value = bvalue;
    value = (value - 0x40) * 100 / 0x40;
    DEBUG_MSG("Bend change: value=", value, ", channel=", channel);

    synth_channel.effect.current_bend = value;

    for (int i=0; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel == synth_channel.midiChannel) {
            bendOsc(i, value);
        }
    }
}

void volumeChange(byte channel, byte value)
{
    if (synthChannels[channel].isNone()) {
        return;
    }
    auto& synth_channel = synthChannels[channel];

    DEBUG_MSG("Volume change: value=", value, ", channel=", channel);

    synth_channel.volume = value;

    for (int i=0; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel == synth_channel.midiChannel) {
            signalOscVolumeChange(i);
        }
    }
}
