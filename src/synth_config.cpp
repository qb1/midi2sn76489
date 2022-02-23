#include <Arduino.h>

#include "config.h"
#include "synth_internals.h"

SynthChannel* synthChannels[16] = {
    SynthChannel::makePolyphonic(1, { .attack = 10, .decay = 200, .sustain = 10, .rel = 500 }, VOICES_COUNT ),
    SynthChannel::makePolyphonic(2, { .attack = 10, .decay = 200, .sustain = 10, .rel = 500 }, 2 ),
    SynthChannel::makeArpeggio  (3, { .attack = 10, .decay = 200, .sustain = 10, .rel = 500 }, 100 ),
    SynthChannel::makeDrum      (4, 2 ),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

// Noise type
#define PNOISE 0b000 // Periodic noise
#define WNOISE 0b100 // White noise

// Frequency control: shift rate
#define N1 0b000 // N/512
#define N2 0b001 // N/1024
#define N3 0b010 // N/2048
// #define T3 0b011 // Tone generator #3 output: not used

// Note: to make use of tone generation 3, we should make a way to somehow control
// these oscillators specifically - we can't today.

DrumDefinition drumDefinitions[12] = {
    { .envelope = { .attack = 10, .decay = 300, .sustain = 0, .rel = 100 }, .noise = WNOISE | N1 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 0, .rel = 100 }, .noise = WNOISE | N2 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 0, .rel = 100 }, .noise = WNOISE | N3 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 0, .rel = 100 }, .noise = PNOISE | N1 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 0, .rel = 100 }, .noise = PNOISE | N2 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 0, .rel = 100 }, .noise = PNOISE | N3 },
    { .envelope = { .attack = 10, .decay = 800, .sustain = 0, .rel = 200 }, .noise = WNOISE | N1 },
    { .envelope = { .attack = 10, .decay = 800, .sustain = 0, .rel = 200 }, .noise = WNOISE | N2 },
    { .envelope = { .attack = 10, .decay = 800, .sustain = 0, .rel = 200 }, .noise = WNOISE | N3 },
    { .envelope = { .attack = 10, .decay = 800, .sustain = 0, .rel = 200 }, .noise = PNOISE | N1 },
    { .envelope = { .attack = 10, .decay = 800, .sustain = 0, .rel = 200 }, .noise = PNOISE | N2 },
    { .envelope = { .attack = 10, .decay = 800, .sustain = 0, .rel = 200 }, .noise = PNOISE | N3 },
};

DrumDefinition& drumDefinitionFromPitch(byte pitch) {
    return drumDefinitions[pitch % (sizeof(drumDefinitions) / sizeof(DrumDefinition))];
}
