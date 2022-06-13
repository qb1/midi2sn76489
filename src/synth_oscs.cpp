#include <Arduino.h>

#include "synth_oscs.h"
#include "config.h"
#include "midi_defs.h"
#include "board.h"
#include "synth_internals.h"

// #define ENABLE_DEBUG_LOGS
#include "logs.h"

struct OscState {
    enum State {
        Off,
        Attack,
        Decay,
        Sustain,
        Release,
        Sample,
    };

	int chip;
    int chip_channel;

	int volume = 0;
    byte velocity = 0;
    byte pitch = 0;

    State state = Off;

    // Volumes corrected for velocity
    byte corrected_attack;
    byte corrected_sustain;

	bool on_going_slope = false;
	int objective = 0;
	int amount_per_step = 0;   // Amount per step
	int ticks_per_step = 0;    // Ticks between steps
	int tick_counter = 0;      // Current tick counter until next step

    byte ticks_since_release = 0;

    struct SampleData {
        const SamplePoint* point_pgm;
        byte until_next = 0;
    };

    SampleData sample;
    Envelope envelope;
    const SynthChannel* channel;
};

OscState oscs[VOICES_COUNT];

// Set voice on volume linear slope, that will be updated periodically until objective volume is reached.
// Will ignore current volume to compute slope but use `from` parameter instead, so that slopes are always identical
// regardless of current state
// Note: do not set the volume after calling thing function, the interrupts will start acting of it.
void setOnVolumeSlope(OscState& v, int slope_from, int slope_to, int actual_to, int ms);

byte getVolumeFromVelocity(byte volume, byte velocity);

void setupSynthOscs()
{
	for (int i=0; i < VOICES_COUNT; ++i) {
		oscs[i] = {
			.chip = i / VOICES_PER_CHIP,
            .chip_channel = i % VOICES_PER_CHIP,
		};
	}
}

void forceOscOff(byte osc) 
{
    OscState& v = oscs[osc % VOICES_COUNT];
    // First, ensure interrupts won't mess with our osc.
    v.on_going_slope = false;
    v.state = OscState::Off;
    v.volume = 0;
    updateVolume(v.chip, v.chip_channel, v.volume);
}

void moveOscToNextModulation(OscState& v)
{
    switch(v.state) {
        case OscState::Off:
            // Setting a oscillator ON is manually done when noteOff
            DEBUG_MSG("Trying to move osc to next modulation from Off");
            break;
        case OscState::Sample:
            // No envelope on samples
            DEBUG_MSG("Trying to move osc to next modulation from Sample");
            break;
        case OscState::Attack:
            v.state = OscState::Decay;
            setOnVolumeSlope(v, 15, v.envelope.sustain, v.corrected_sustain, v.envelope.decay);
            break;
        case OscState::Decay:
            v.state = OscState::Sustain;
            // Keep volume as it is
            break;
        case OscState::Sustain:
            // Setting a oscillator to Release is manually done when noteOff
            DEBUG_MSG("Trying to move osc to next modulation from Release");
            break;
        case OscState::Release:
            v.state = OscState::Off;
            break;
    }
    updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));
}

void updateSamplePlaying(OscState& v)
{
    if (v.sample.until_next < REFRESH_RATE) {
        ++v.sample.point_pgm;
        SamplePoint sample_point;
        memcpy_P(&sample_point, v.sample.point_pgm, sizeof(SamplePoint));

        if (sample_point.freq_n == 0) {
            // Is Off
            v.state = OscState::Off;
            v.sample.point_pgm = nullptr;
            v.sample.until_next = 0;
            updateVolume(v.chip, v.chip_channel - 1, 0);
        }
        
        // HACK HACK: osc noise+2
        updateFreq(v.chip, v.chip_channel - 1, sample_point.freq_n);
        auto volume = v.channel->correct_volume(getVolumeFromVelocity(sample_point.amplitude, v.velocity));
        updateVolume(v.chip, v.chip_channel - 1, volume);
        DEBUG_MSG("Sample: t=", v.sample.until_next, ", to ", sample_point.freq_n, ":", volume, " for ", sample_point.ms);
        v.sample.until_next = sample_point.ms;
    } else {
        v.sample.until_next -= REFRESH_RATE;
    }
}

void updateSynthOscs()
{
	for (int i=0; i < VOICES_COUNT; ++i){
        OscState& v = oscs[i];

        if (v.state == OscState::Sample) {
            updateSamplePlaying(v);
            continue;
        }

        if (not v.on_going_slope) {
            continue;
        }

        if (v.state == OscState::Release) {
            if (v.ticks_since_release < 255) {
                v.ticks_since_release += 1;
            }
        }

        v.tick_counter -= 1;
        if (v.tick_counter <= 0) {
            v.tick_counter = v.ticks_per_step;

            v.volume += v.amount_per_step;
            if ((v.amount_per_step < 0 and v.volume <= v.objective) or (v.amount_per_step > 0 and v.volume >= v.objective) or
                (v.amount_per_step == 0)) {
                v.volume = v.objective;
                v.on_going_slope = false;

                updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));
                moveOscToNextModulation(v);
            } else {
                updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));
            }
        }
    }
}

void setOnVolumeSlope(OscState& v, int slope_from, int slope_to, int actual_to, int ms)
{
    // Ensure slope is disabled first, interrupts might still be firing
    v.on_going_slope = false;
    DEBUG_MSG("Setting slope y=x+(", slope_to - slope_from, ") until ", actual_to, " in ", ms);

    if ((slope_from < slope_to && v.volume >= actual_to) || (slope_to <  slope_from and v.volume <= actual_to) ||
        (ms < REFRESH_RATE)) {
        DEBUG_MSG("Where we're going we don't need slopes");

        // No slope possible or necessary
        v.volume = actual_to;
        updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));

        // Still set the slope to keep the oscillator in the modulation interrupts as expected by caller:
        // might get moved to next mod state on the next tick
        v.tick_counter = 2; // At least one full tick
        v.ticks_per_step = 1;
        v.amount_per_step = 0;
        v.objective = v.volume;
        v.on_going_slope = true;
        return;
    }

    int steps_count = ms / REFRESH_RATE;
    int amount = actual_to - slope_from;

    if (abs(amount) >= steps_count) {
		v.ticks_per_step = 1;
		v.amount_per_step = amount / steps_count;
	} else {
		v.amount_per_step = amount > 0 ? 1 : -1;
		v.ticks_per_step = abs(steps_count / amount);
	}

	v.tick_counter = v.ticks_per_step;
    v.objective = actual_to;

    DEBUG_MSG("Slope characs: ", v.ticks_per_step, " ticks/step ", v.amount_per_step, " volume/step ");

    // Finally, activate refresh on this
    v.on_going_slope = true;
}

byte getVolumeFromVelocity(byte volume, byte velocity)
{
    if (volume == 0)
        return 0;
    return max((unsigned int)(volume) * (unsigned int)velocity / 127, 1);
}

void startOsc(byte osc, byte pitch, byte velocity, const SynthChannel& channel, const Envelope& envelope, bool pitch_is_noise_control) {
    OscState& v = oscs[osc % VOICES_COUNT];

    DEBUG_MSG("Start osc ", osc);

    // Fist step is to disable the slope, so the interrupts won't mess with our volume
	v.on_going_slope = false;
    // Do not set the volume to 0 - this creates nasty cracks if the track is recycled.
    // On the other hand, it means a recycled oscillator won't get a full attack, but that's
    // a better compromise.

    v.channel = &channel;
    v.envelope = envelope;
    v.corrected_attack = getVolumeFromVelocity(15, velocity);
    v.corrected_sustain = getVolumeFromVelocity(v.envelope.sustain, velocity);
    v.pitch = pitch;
    v.velocity = velocity;

    if (pitch_is_noise_control) {
        // HACK HACK: osc noise+2
        updateVolume(v.chip, v.chip_channel - 1, 0);
        updateFreq(v.chip, v.chip_channel - 1, drumDefinitionFromPitch(pitch).osc3freq);        
        updateNoise(v.chip, drumDefinitionFromPitch(pitch).noise);
    } else {
        updateFreq(v.chip, v.chip_channel, pitch_value(pitch));
    }

    // Start the oscillator (or set the slope)
    v.state = OscState::Attack;
    setOnVolumeSlope(v, 0, 15, v.corrected_attack, envelope.attack);
}

void startOsc(byte osc, byte pitch, byte velocity, const SynthChannel& channel, const SamplePoint* sample)
{
    OscState& v = oscs[osc % VOICES_COUNT];

    DEBUG_MSG("Start osc sample ", osc);

    // Fist step is to disable the slope, so the interrupts won't mess with our volume
	v.on_going_slope = false;
    v.channel = &channel;
    v.envelope = Envelope();
    v.corrected_attack = 0;
    v.corrected_sustain = 0;
    v.pitch = pitch;
    v.velocity = velocity;

    // Start the oscillator (or set the slope)
    v.sample.point_pgm = sample;
    SamplePoint sample_point;
    memcpy_P(&sample_point, v.sample.point_pgm, sizeof(SamplePoint));

    v.sample.until_next = sample_point.ms;

    // HACK HACK: osc noise+2
    updateVolume(v.chip, v.chip_channel, 0); // clicks :(
    updateFreq(v.chip, v.chip_channel - 1, sample_point.freq_n);
    auto volume = v.channel->correct_volume(getVolumeFromVelocity(sample_point.amplitude, v.velocity));
    updateVolume(v.chip, v.chip_channel - 1, volume);
    DEBUG_MSG("Start osc ", osc, ", at ", sample_point.freq_n, ":", volume, " for ", sample_point.ms);
    v.state = OscState::Sample;    
}

void moveOsc(byte osc, byte pitch)
{
    OscState& v = oscs[osc % VOICES_COUNT];

    v.pitch = pitch;

    updateFreq(v.chip, v.chip_channel, pitch_value(pitch));
    if (v.state == OscState::Release) {
        // Was set to release, take it back to sustain
        v.on_going_slope = false;
        v.state = OscState::Sustain;
        v.volume = v.corrected_sustain;
        updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));
    }
}

void bendOsc(byte osc, int bend, int semitones)
{
    OscState& v = oscs[osc % VOICES_COUNT];

    if ((v.pitch == 0 && bend < 0) || (v.pitch == LAST_NOTE && bend > 0)) {
        return;
    }
    int32_t current = pitch_value(v.pitch);
    int32_t next    = pitch_value(v.pitch + (bend > 0 ? semitones : -semitones));
    int32_t diff = (next - current) * abs(bend) / 100;
    int32_t bent = current + diff;

    DEBUG_MSG("Bend osc ", bend, " from ", current, " to ", bent);

    updateFreq(v.chip, v.chip_channel, bent);
}

void signalOscVolumeChange(byte osc)
{
    OscState& v = oscs[osc % VOICES_COUNT];
    updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));
}

void tremoloOsc(byte osc, int value)
{
    OscState& v = oscs[osc % VOICES_COUNT];

    if (v.state == OscState::Off) {
        // I only applied tremolo to sustain first, but that made it weird or useless as simple timbre
        // effect for eg. filler notes, so let's apply it everywhere.
        return;
    }
    int volume = v.volume + 15 * value / 100;

    updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(volume));
}

void stopOsc(byte osc) {
    OscState& v = oscs[osc % VOICES_COUNT];

    if (v.state == OscState::Sample) {
        // Not handled for now.
        return;
    }

    v.state = OscState::Release;
    // If no sustain, release slope will apply similarly to decay slope, from the top
    unsigned int slope_start_volume =  v.envelope.sustain > 0 ? v.envelope.sustain : 15;
    v.ticks_since_release = 0;
	setOnVolumeSlope(v, slope_start_volume, 0, 0, v.envelope.rel);
}

bool isOscActive(byte osc)
{
    OscState& v = oscs[osc % VOICES_COUNT];
	return v.state != OscState::Off;
}

bool isOscReleasing(byte osc)
{
    OscState& v = oscs[osc % VOICES_COUNT];
	return v.state == OscState::Release;
}

bool isOscLegatoReady(byte osc)
{
    OscState& v = oscs[osc % VOICES_COUNT];

    if (v.state != OscState::Release) {
        return v.state != OscState::Off;
    }
	return v.ticks_since_release < 2;
}

int oscTargetVolume(byte osc)
{
    OscState& v = oscs[osc % VOICES_COUNT];
	return v.on_going_slope ? v.objective : v.volume;
}
