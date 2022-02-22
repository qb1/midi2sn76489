#include <Arduino.h>

#include "config.h"
#include "synth_internals.h"

SynthChannel* synthChannels[16] = {
    SynthChannel::makePolyphonic(0, { .rel = 500 }, VOICES_COUNT ),
    SynthChannel::makePolyphonic(1, { .rel = 500 }, 1 ),
    SynthChannel::makeArpeggio  (2, { .rel = 500 }, 100 ),
    SynthChannel::makeDrum      (3, 2 ),
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
