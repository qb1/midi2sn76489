#include <Arduino.h>

#include "synth.h"
#include "config.h"
#include "synth_voices.h"

static unsigned int priorityCounter = 0;

struct VoiceProperties {
    unsigned int priority;

    byte channel;
    byte pitch;
};

VoiceProperties voice_properties[VOICES_COUNT];

byte findAvailableVoice();
void updateVoiceProperties(byte voice, byte channel, byte pitch);
void resetVoiceProperties(byte voice);

void setupSynth() {
    setupSynthVoices();
    for (int i=0; i < VOICES_COUNT; ++i) {
        resetVoiceProperties(i);
    }
}

void updateSynth() {
    updateSynthVoices();
}

byte findAvailableVoice()
{
    unsigned int lowest_prio = -1;
    byte lowest_prio_voice = 0xff;

    for (int i=0; i < VOICES_COUNT; ++i) {
        if (i % 4 == 3) // Avoid noises channels for now
            continue;

        if (not isVoiceActive(i)) {
            return i;
        }

        if (voice_properties[i].priority <= lowest_prio) {
            lowest_prio = voice_properties[i].priority;
            lowest_prio_voice = i;
        }
    }
    return lowest_prio_voice;
}

void updateVoiceProperties(byte voice, byte channel, byte pitch)
{
    voice_properties[voice] = {
        .priority = priorityCounter++,
        .channel = channel,
        .pitch = pitch,
    };
}

void resetVoiceProperties(byte voice)
{
    voice_properties[voice] = {
        .priority = 0,
        .channel = 0xff,
        .pitch = 0xff,
    };    
}

byte findVoice(byte channel, byte pitch)
{
    for (int i=0; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel == channel && 
            voice_properties[i].pitch == pitch) {
            return i;
        }
    }
    return 0xff;
}

void noteOn(byte channel, byte pitch, byte velocity) {	
	Serial.print("noteOn channel=");
	Serial.print(channel);
	Serial.print(", pitch=");
	Serial.println(pitch);
	
	byte voice = findAvailableVoice();
    if (voice == 0xff) {
        Serial.println("No available voice");
        return;
    }
    
    Serial.print("Starting voice");
    Serial.println(voice);

    updateVoiceProperties(voice, channel, pitch);
    startVoice(voice, pitch);
}

void noteOff(byte channel, byte pitch, byte velocity) {
    byte voice = findVoice(channel, pitch);
     while (voice != 0xff) {       
        Serial.print("Stopping voice");
        Serial.println(voice);

        stopVoice(voice);
        resetVoiceProperties(voice);

        voice = findVoice(channel, pitch); 
    }
}

void controlChange(byte channel, byte control, byte value) {
	Serial.print("Control change: control=");
	Serial.print(control);
	Serial.print(", value=");
	Serial.print(value);
	Serial.print(", channel=");
	Serial.println(channel);
}
