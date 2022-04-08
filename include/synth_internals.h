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
        Arpeggio,
    };

    uint8_t type = None;
    int speed = 0;
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

    const byte midiChannel;

    // Restrict which chips this voice can be on
    const byte onChip;

    static SynthChannel makeDrum(int midiChannel, int onChip, const int voiceCount) {
        return SynthChannel(Drum, midiChannel, onChip, Envelope(), VoiceEffect(), voiceCount);
    }

    static SynthChannel makePolyphonic(int midiChannel, int onChip, const Envelope& envelope, const int voiceCount) {
        return SynthChannel(Music, midiChannel, onChip, envelope, VoiceEffect(), voiceCount);
    }

    static SynthChannel makeArpeggio(int midiChannel, int onChip, const Envelope& envelope, const int speed) {
        return SynthChannel(Music, midiChannel, onChip, envelope, { .type = VoiceEffect::Arpeggio, .speed = speed }, 1);
    }

    static SynthChannel makeNone() {
        return SynthChannel(Music, 0, 0, Envelope(), VoiceEffect(), 0);
    }

    bool isNone() const { return voiceCount == 0; } // No optional :(

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
