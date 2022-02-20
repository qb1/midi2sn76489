#include <Arduino.h>

#include "control.h"
#include "config.h"
#include "midi_defs.h"
#include "board.h"

void updateChannel(struct ChannelState& channel);
void startChannel(const struct Envelope& env, struct ChannelState& channel);
void stopChannel(const struct Envelope& env, struct ChannelState& channel);
void setChannelRelease(const struct Envelope& env, struct ChannelState& channel);

struct Envelope {
	unsigned int rel;
};

struct ChannelState {
	int channel_nb;
	
	int volume;

	bool on_going;
	int objective;
	
	int step_offset;
	int step_ticks;
	int ticks_to_step;
};

struct Envelope envelope[4];
struct ChannelState voice[4];

void setupControl()
{
	for (int i=0; i < 4; ++i) {
		voice[i] = { 
			.channel_nb = i, 
			.volume = 0, 
			.on_going = false, 
			.objective = 0, 
			.step_offset = 0, 
			.step_ticks = 0, 
			.ticks_to_step = 0 
		};
		envelope[i] = { .rel = 1000 };
	}
	
	/*selectChip(0);
	noteOn(0, 69, 0);
	selectChip(1);
	noteOn(0, 61, 0);
	selectChip(2);
	noteOn(0, 64, 0);
	selectChip(3);
	noteOn(0, 48, 0);*/
}

void updateChannel(struct ChannelState& channel) {
	if (not channel.on_going) {
		return;
	}
	
	channel.ticks_to_step -= 1;
	if (channel.ticks_to_step <= 0) {
		channel.ticks_to_step = channel.step_ticks;
		
		channel.volume += channel.step_offset;
		if ((channel.step_offset < 0 and channel.volume <= channel.objective) or (channel.step_offset > 0 and channel.volume >= channel.objective)) {
			channel.volume = channel.objective;
			channel.on_going = false;
		}
		updateVolume(channel.channel_nb, channel.volume);
	}
}

void startChannel(const struct Envelope& env, struct ChannelState& channel) {
	channel.volume = 15;
	channel.on_going = false;
	updateVolume(channel.channel_nb, channel.volume);
}

void stopChannel(const struct Envelope& env, struct ChannelState& channel) {
	setChannelRelease(env, channel);
}

void setChannelRelease(const struct Envelope& env, struct ChannelState& channel) {
	channel.on_going = true;
	
	channel.objective = 0;
	int steps_count = env.rel / REFRESH_RATE;
	int amount = channel.objective - channel.volume;
	if (abs(amount) >= steps_count) {
		channel.step_ticks = 1;
		channel.step_offset = amount / steps_count;     
	} else {
		channel.step_offset = amount > 0 ? 1 : -1;
		channel.step_ticks = abs(steps_count / amount);
	}

	channel.ticks_to_step = channel.step_ticks;  

	/*Serial.print("Channel update: ");
	Serial.print(steps_count); Serial.print(" ");
	Serial.print(amount); Serial.print(" ");
	Serial.print(channel.objective); Serial.print(" ");
	Serial.print(channel.step_offset); Serial.print(" ");
	Serial.print(channel.step_ticks); Serial.print(" ");
	Serial.println("");*/
}


void noteOn(byte channel, byte pitch, byte velocity) {
	if (pitch < FIRST_NOTE) {
		Serial.print("TOO LOW");
		return;
	}
	if (pitch > LAST_NOTE) {
		Serial.print("TOO HIGH");
		return;
	}
	
	Serial.print("noteOn channel=");
	Serial.print(channel);
	Serial.print(", pitch=");
	Serial.println(pitch);
	
	if (channel <= 3) {
		updateFreq(channel, NOTES[pitch]);
		startChannel(envelope[channel], voice[channel]);
	}
	
}

void noteOff(byte channel, byte pitch, byte velocity) {
	stopChannel(envelope[channel], voice[channel]);
}

void controlChange(byte channel, byte control, byte value) {
	Serial.print("Control change: control=");
	Serial.print(control);
	Serial.print(", value=");
	Serial.print(value);
	Serial.print(", channel=");
	Serial.println(channel);
}


void updateChannels()
{
	for (int i=0; i < 4; ++i)
		updateChannel(voice[i]);
}
