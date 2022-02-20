#include <Arduino.h>

#include "synth_voices.h"
#include "config.h"
#include "midi_defs.h"
#include "board.h"

struct Envelope {
	unsigned int rel;
};

struct VoiceState {
	int chip;
    int chip_channel;
	
	int volume;

	bool on_going_slope;
	int objective;
	
	int step_offset;
	int step_ticks;
	int ticks_to_step;

    Envelope envelope;
};

VoiceState voices[VOICES_COUNT];

void setupSynthVoices()
{
	for (int i=0; i < VOICES_COUNT; ++i) {
		voices[i] = { 
			.chip = i / VOICES_PER_CHIP,
            .chip_channel = i % VOICES_PER_CHIP,
			.volume = 0, 
			.on_going_slope = false, 
			.objective = 0, 
			.step_offset = 0, 
			.step_ticks = 0, 
			.ticks_to_step = 0,
            .envelope = { .rel = 1000 },
		};
	}
}

void updateSynthVoices()
{
	for (int i=0; i < VOICES_COUNT; ++i){
        VoiceState& v = voices[i];

        if (not v.on_going_slope) {
            continue;
        }
        
        v.ticks_to_step -= 1;
        if (v.ticks_to_step <= 0) {
            v.ticks_to_step = v.step_ticks;
            
            v.volume += v.step_offset;
            if ((v.step_offset < 0 and v.volume <= v.objective) or (v.step_offset > 0 and v.volume >= v.objective)) {
                v.volume = v.objective;
                v.on_going_slope = false;
            }

            selectChip(v.chip);
            updateVolume(v.chip_channel, v.volume);
        }
    }
}

void startVoice(byte voice, byte pitch) {
    VoiceState& v = voices[voice % VOICES_COUNT];
	v.volume = 15; // Max volume
	v.on_going_slope = false;
    selectChip(v.chip);
    updateFreq(v.chip_channel, NOTES[pitch]);
	updateVolume(v.chip_channel, v.volume);
}

void stopVoice(byte voice) {
    VoiceState& v = voices[voice % VOICES_COUNT];
	v.on_going_slope = true;
	
	v.objective = 0;
	int steps_count = v.envelope.rel / REFRESH_RATE;
	int amount = v.objective - v.volume;
	if (abs(amount) >= steps_count) {
		v.step_ticks = 1;
		v.step_offset = amount / steps_count;     
	} else {
		v.step_offset = amount > 0 ? 1 : -1;
		v.step_ticks = abs(steps_count / amount);
	}

	v.ticks_to_step = v.step_ticks;  
}

bool isVoiceActive(byte voice)
{
    VoiceState& v = voices[voice % VOICES_COUNT];
	return v.volume != 0;
}

