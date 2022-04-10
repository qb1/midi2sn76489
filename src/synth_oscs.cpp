#include <Arduino.h>

#include "synth_oscs.h"
#include "config.h"
#include "midi_defs.h"
#include "board.h"
#include "synth_internals.h"

// #define ENABLE_DEBUG_LOGS // Careful: some debug logs during interrupt, might get unstable
#include "logs.h"

struct OscState {
    enum EnvelopeState {
        Off,
        Attack,
        Decay,
        Sustain,
        Release,
    };

	int chip;
    int chip_channel;

	int volume = 0;
    byte pitch = 0;

    EnvelopeState state = Off;

    // Volumes corrected for velocity
    byte corrected_attack;
    byte corrected_sustain;

	bool on_going_slope = false;
	int objective = 0;
	int amount_per_step = 0;   // Amount per step
	int ticks_per_step = 0;    // Ticks between steps
	int tick_counter = 0;      // Current tick counter until next step

    byte ticks_since_release = 0;

    Envelope envelope;
    const SynthChannel* channel;
};

OscState oscs[VOICES_COUNT];

// Set voice on volume linear slope, that will be updated periodically until objective volume is reached.
// Will ignore current volume to compute slope but use `from` parameter instead, so that slopes are always identical
// regardless of current state
// Note: do not set the volume after calling thing function, the interrupts will start acting of it.
void setOnVolumeSlope(OscState& v, int slope_from, int slope_to, int actual_to, int ms);

// Force osc off, regardless of state, and set state to Off.
// Might cause annoying clicks
void forceOscOff(OscState& v);

void setupSynthOscs()
{
	for (int i=0; i < VOICES_COUNT; ++i) {
		oscs[i] = {
			.chip = i / VOICES_PER_CHIP,
            .chip_channel = i % VOICES_PER_CHIP,
		};
	}
}

void forceOscOff(OscState& v)
{
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

void updateSynthOscs()
{
	for (int i=0; i < VOICES_COUNT; ++i){
        OscState& v = oscs[i];

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
        v.tick_counter = 1;
        v.ticks_per_step = 1;
        v.amount_per_step = 0;
        v.objective = v.volume;
        v.on_going_slope = true;
        return;
    }

    int steps_count = ms / REFRESH_RATE;
    int amount = slope_to - slope_from;

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
    return (unsigned int)(volume) * velocity / 127;
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

    if (pitch_is_noise_control) {
        updateFreq(v.chip, 2, drumDefinitionFromPitch(pitch).osc3freq);
        updateNoise(v.chip, drumDefinitionFromPitch(pitch).noise);
    } else {
        updateFreq(v.chip, v.chip_channel, pitch_value(pitch));
    }

    // Start the oscillator (or set the slope)
    v.state = OscState::Attack;
    setOnVolumeSlope(v, 0, 15, v.corrected_attack, envelope.attack);
}

void moveOsc(byte osc, byte pitch, bool pitch_is_noise_control)
{
    OscState& v = oscs[osc % VOICES_COUNT];

    v.pitch = pitch;

    if (pitch_is_noise_control) {
        updateFreq(v.chip, 2, drumDefinitionFromPitch(pitch).osc3freq);
        updateNoise(v.chip, drumDefinitionFromPitch(pitch).noise);
    } else {        
        updateFreq(v.chip, v.chip_channel, pitch_value(pitch));
        if (v.state == OscState::Release) {
            // Was set to release, take it back to sustain
            v.on_going_slope = false;
            v.state = OscState::Sustain;
            v.volume = v.corrected_sustain;
            updateVolume(v.chip, v.chip_channel, v.channel->correct_volume(v.volume));
        }
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

void stopOsc(byte osc) {
    OscState& v = oscs[osc % VOICES_COUNT];

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
