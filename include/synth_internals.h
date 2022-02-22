#include <Arduino.h>

/*
 * Define internal synths objects
 */

struct Envelope {
	unsigned int rel;
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
    Envelope envelope;
    int voiceCount;
    VoiceEffect effect; // No effect allowed if voiceCount > 1

    const int midiChannel;

    static SynthChannel* makePolyphonic(int midiChannel, const Envelope& envelope, const int voiceCount) {
        return new SynthChannel(midiChannel, envelope, VoiceEffect(), voiceCount);
    }

    static SynthChannel* makeArpeggio(int midiChannel, const Envelope& envelope, const int speed) {
        return new SynthChannel(midiChannel, envelope, { .type = VoiceEffect::Arpeggio, .speed = speed }, 1);
    }

private:
    SynthChannel(int midiChannel, const Envelope& envelope, const VoiceEffect& effect, const int voiceCount)
      : envelope(envelope)
      , voiceCount(voiceCount)
      , effect(effect)
      , midiChannel(midiChannel)
    {
    }
};

extern SynthChannel* synthChannels[16];
