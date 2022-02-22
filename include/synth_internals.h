#pragma once
#include <Arduino.h>

/*
 * Define internal synths objects
 */

struct Envelope {
	unsigned int rel = 0;
};

struct VoiceEffect {
    enum Type {
        None,
        Arpeggio,
    };

    Type type = None;
    int speed = 0;
};

struct SynthChannel {
    enum Type {
        Music,
        Drum
    };

    Type type;
    Envelope envelope;
    int voiceCount;
    VoiceEffect effect; // No effect allowed if voiceCount > 1

    const int midiChannel;

    static SynthChannel* makeDrum(int midiChannel, const int voiceCount) {
        return new SynthChannel(Drum, midiChannel, Envelope(), VoiceEffect(), voiceCount);
    }

    static SynthChannel* makePolyphonic(int midiChannel, const Envelope& envelope, const int voiceCount) {
        return new SynthChannel(Music, midiChannel, envelope, VoiceEffect(), voiceCount);
    }

    static SynthChannel* makeArpeggio(int midiChannel, const Envelope& envelope, const int speed) {
        return new SynthChannel(Music, midiChannel, envelope, { .type = VoiceEffect::Arpeggio, .speed = speed }, 1);
    }

private:
    SynthChannel(Type type, int midiChannel, const Envelope& envelope, const VoiceEffect& effect, const int voiceCount)
      : type(type)
      , envelope(envelope)
      , voiceCount(voiceCount)
      , effect(effect)
      , midiChannel(midiChannel)
    {
    }
};

extern SynthChannel* synthChannels[16];
