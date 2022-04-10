#include <Arduino.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "synth_internals.h"

#define VOICE_0 (0b0001)
#define VOICE_1 (0b0010)
#define VOICE_2 (0b0100)
#define VOICE_3 (0b1000)

#define VOICE_R (0b0011)
#define VOICE_L (0b1100)

SynthChannel synthChannels[16] = {
    SynthChannel::makePolyphonic(0, VOICE_0, { .attack = 5, .decay = 200, .sustain = 12, .rel = 500 }, 2 ),
    SynthChannel::makePolyphonic(1, VOICE_1, { .attack = 5, .decay = 200, .sustain = 12, .rel = 200 }, 1 ),
    SynthChannel::makePolyphonic(2, VOICE_1, { .attack = 5, .decay = 200, .sustain = 12, .rel = 200 }, 1 ),

    SynthChannel::makePolyphonic(3, VOICE_2, { .attack = 5, .decay = 200, .sustain = 11, .rel = 500 }, 2 ),
    SynthChannel::makePolyphonic(4, VOICE_3, { .attack = 5, .decay = 500, .sustain = 12, .rel = 200 }, 1 ),
    SynthChannel::makePolyphonic(5, VOICE_3, { .attack = 5, .decay = 500, .sustain = 12, .rel = 200 }, 1 ),

    // Full polyphony
    SynthChannel::makePolyphonic(6, VOICE_L, { .attack = 20, .decay = 200, .sustain = 12, .rel = 500 }, 4 ),
    SynthChannel::makePolyphonic(7, VOICE_R, { .attack = 20, .decay = 200, .sustain = 12, .rel = 500 }, 4 ),
    SynthChannel::makeNone(),

    SynthChannel::makeNone(),
    SynthChannel::makeNone(),
    SynthChannel::makeNone(),

    SynthChannel::makeNone(),
    SynthChannel::makeDrum(13, VOICE_L, 2 ),
    SynthChannel::makeDrum(14, VOICE_R, 2 ),

    SynthChannel::makeNone(),
   
    /*// Nice note
    SynthChannel::makePolyphonic(0, VOICE_0, { .attack = 5, .decay = 200, .sustain = 8, .rel = 500 }, 2 ),
    SynthChannel::makePolyphonic(1, VOICE_1, { .attack = 5, .decay = 100, .sustain = 12, .rel = 200 }, 1 ),
    SynthChannel::makePolyphonic(2, VOICE_1, { .attack = 5, .decay = 100, .sustain = 14, .rel = 250 }, 1 ),

    SynthChannel::makePolyphonic(3, VOICE_2, { .attack = 10, .decay = 200, .sustain = 13, .rel = 500 }, 2 ),
    SynthChannel::makePolyphonic(4, VOICE_3, { .attack = 5, .decay = 100, .sustain = 12, .rel = 200 }, 1 ),
    SynthChannel::makePolyphonic(5, VOICE_3, { .attack = 5, .decay = 100, .sustain = 12, .rel = 200 }, 1 ),

    // Direct control, no attack
    SynthChannel::makePolyphonic(6, VOICE_0, { .attack = 10, .decay = 0, .sustain = 15, .rel = 500 }, 2 ),
    SynthChannel::makePolyphonic(7, VOICE_1, { .attack = 10, .decay = 0, .sustain = 15, .rel = 500 }, 1 ),
    SynthChannel::makePolyphonic(8, VOICE_1, { .attack = 10, .decay = 0, .sustain = 15, .rel = 500 }, 1 ),

    SynthChannel::makePolyphonic(9,  VOICE_2, { .attack = 10, .decay = 0, .sustain = 15, .rel = 500 }, 2 ),
    SynthChannel::makePolyphonic(10, VOICE_3, { .attack = 1, .decay = 0, .sustain = 15, .rel = 50 }, 1 ),
    SynthChannel::makePolyphonic(11, VOICE_3, { .attack = 10, .decay = 0, .sustain = 15, .rel = 500 }, 1 ),

    SynthChannel::makeNone(),
    SynthChannel::makeDrum      (13, VOICE_L, 2 ),
    SynthChannel::makeDrum      (14, VOICE_R, 2 ),

    SynthChannel::makePolyphonic(15, VOICE_R, { .attack = 20, .decay = 200, .sustain = 8, .rel = 500 }, 4 )*/
};

// Noise type
#define PNOISE 0b000 // Periodic noise
#define WNOISE 0b100 // White noise

// Frequency control: shift rate
#define N1 0b000 // N/512
#define N2 0b001 // N/1024
#define N3 0b010 // N/2048
#define T3 0b011 // Tone generator #3 output

const DrumDefinition drumDefinitions[12] = {
    { .envelope = { .attack =  0, .decay = 200, .sustain =  0, .rel = 200 }, .noise = WNOISE | T3, .osc3freq =   1 }, // closed hit-hat
    { .envelope = { .attack = 10, .decay = 300, .sustain =  0, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   1 }, // open hit-hat
    { .envelope = { .attack = 10, .decay =  25, .sustain =  8, .rel = 500 }, .noise = WNOISE | T3, .osc3freq =   1 }, // crash
    { .envelope = { .attack = 10, .decay = 350, .sustain =  0, .rel = 350 }, .noise = WNOISE | T3, .osc3freq =  30/4 }, // snare
    { .envelope = { .attack = 10, .decay = 350, .sustain =  0, .rel = 350 }, .noise = WNOISE | T3, .osc3freq = 120/4 }, // kick
    { .envelope = { .attack = 10, .decay = 350, .sustain =  0, .rel = 350 }, .noise = WNOISE | T3, .osc3freq = 160/4 }, // low kick
    { .envelope = { .attack = 10, .decay = 350, .sustain =  0, .rel = 350 }, .noise = PNOISE | T3, .osc3freq = 179/4 }, // musical kick
    { .envelope = { .attack = 10, .decay = 350, .sustain =  0, .rel = 350 }, .noise = PNOISE | T3, .osc3freq = 30/4 }, // ?
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
    { .envelope = { .attack = 10, .decay = 600, .sustain = 15, .rel = 300 }, .noise = WNOISE | T3, .osc3freq =   0 }, //
};

const DrumDefinition& drumDefinitionFromPitch(byte pitch) {
    return drumDefinitions[pitch % (sizeof(drumDefinitions) / sizeof(DrumDefinition))];
}
