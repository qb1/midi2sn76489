#include <Arduino.h>

#include "config.h"
#include "synth_internals.h"

#define VOICE_R (0b0011)
#define VOICE_L (0b1100)

SynthChannel* synthChannels[16] = {
    SynthChannel::makePolyphonic(0, VOICE_R, { .attack = 10, .decay = 200, .sustain = 10, .rel = 500 }, 4 ),
    SynthChannel::makePolyphonic(1, VOICE_L, { .attack = 10, .decay = 200, .sustain = 10, .rel = 500 }, 4 ),
    SynthChannel::makeArpeggio  (2, VOICE_R, { .attack = 10, .decay = 200, .sustain = 10, .rel = 500 }, 100 ),
    SynthChannel::makeDrum      (3, VOICE_L, 2 ),
    SynthChannel::makePolyphonic(1, VOICE_R, { .attack = 10, .decay = 0, .sustain = 15, .rel = 0 }, 2 ),
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
#define T3 0b011 // Tone generator #3 output

DrumDefinition drumDefinitions[12] = {
    { .envelope = { .attack =  0, .decay = 200, .sustain =  0, .rel = 200 }, .noise = WNOISE | T3, .osc3freq =   2 }, // closed hit-hat
    { .envelope = { .attack = 20, .decay = 300, .sustain =  0, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   2 }, // open hit-hat
    { .envelope = { .attack = 20, .decay =  25, .sustain =  8, .rel = 500 }, .noise = WNOISE | T3, .osc3freq =   2 }, // crash
    { .envelope = { .attack = 20, .decay = 350, .sustain =  0, .rel = 350 }, .noise = WNOISE | T3, .osc3freq =  30 }, // snare
    { .envelope = { .attack = 20, .decay = 350, .sustain =  0, .rel = 350 }, .noise = WNOISE | T3, .osc3freq = 120 }, // kick
    { .envelope = { .attack = 20, .decay = 350, .sustain =  0, .rel = 350 }, .noise = WNOISE | T3, .osc3freq = 160 }, // low kick
    { .envelope = { .attack = 20, .decay = 350, .sustain =  0, .rel = 350 }, .noise = PNOISE | T3, .osc3freq = 179 }, // musical kick
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
};

/*DrumDefinition drumDefinitions[12] = {
    { .envelope = { .attack = 10, .decay = 300, .sustain = 15, .rel = 200 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 15, .rel = 200 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 15, .rel = 200 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 15, .rel = 200 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 15, .rel = 200 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 300, .sustain = 15, .rel = 200 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3 },
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3 },
};*/

DrumDefinition& drumDefinitionFromPitch(byte pitch) {
    return drumDefinitions[pitch % (sizeof(drumDefinitions) / sizeof(DrumDefinition))];
}
