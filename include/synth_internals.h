#pragma once
#include <Arduino.h>

/*
 * Define internal synths objects
 */

struct Envelope {
    unsigned short int attack = 0;
    unsigned short int decay = 0;
    unsigned short int sustain = 15;
    unsigned short int rel = 0;
};

struct VoiceEffect {
    enum Type {
        None,
    };

    uint8_t type = None;
    byte legato = true;

    struct Portamento {
        uint16_t speed = 0;
        uint16_t position = 0xffff;
        byte from_pitch = 0;
    };
    Portamento portamento;

    int8_t current_bend = 0;

    struct Vibrato {
        int8_t amount = 0;    // +/- 100 = +/-2 semitones
        uint16_t speed = 200;
        uint16_t position = 0;
    };
    Vibrato vibrato;
};

struct SynthChannel {
    enum Type {
        Music,
        Drum
    };

    const uint8_t type;
    Envelope envelope;
    const uint8_t voiceCount;
    VoiceEffect effect; // Must be None if voiceCount > 1
    byte volume = 127; // From 0 to 127

    const byte midiChannel;

    // Restrict which chips this voice can be on
    const byte onChip;

    static SynthChannel makeDrum(int midiChannel, int onChip, const int voiceCount) {
        return SynthChannel(Drum, midiChannel, onChip, Envelope(), VoiceEffect(), voiceCount);
    }

    static SynthChannel makePolyphonic(int midiChannel, int onChip, const Envelope& envelope, const int voiceCount) {
        return SynthChannel(Music, midiChannel, onChip, envelope, VoiceEffect(), voiceCount);
    }
    
    static SynthChannel makeNone() {
        return SynthChannel(Music, 0, 0, Envelope(), VoiceEffect(), 0);
    }

    bool isNone() const { return voiceCount == 0; } // No optional :(

    byte correct_volume(byte value) const { return (uint16_t)value * volume / 127; }

private:
    SynthChannel(Type type, int midiChannel, int onChip, const Envelope& envelope, const VoiceEffect& effect, const int voiceCount)
      : type(type)
      , envelope(envelope)
      , voiceCount(voiceCount)
      , effect(effect)
      , midiChannel(midiChannel)
      , onChip(onChip)
    {
    }
};

struct DrumDefinition {
    Envelope envelope;
    byte noise;
    byte osc3freq;
};

extern SynthChannel synthChannels[16];
extern const DrumDefinition drumDefinitions[12];

const DrumDefinition& drumDefinitionFromPitch(byte pitch);
