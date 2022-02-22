#include <Arduino.h>

#include "config.h"
#include "synth_oscs.h"
#include "synth_voices.h"

VoiceProperties voice_properties[VOICES_COUNT];

static unsigned int priorityCounter = 0;

byte findAvailableMusicVoice(int channel, int maxPerChannel)
{
    int activeVoicesCount = 0;

    // Will store first found fully available voice.
    // Will be selected if there are no more than maxPerChannel voices used already.
    byte available_voice = 0xff;

    // While exploring, also keep track of track we could recycle early if necessary.
    unsigned int lowest_prio = -1;
    int lowest_volume = 0xff;
    byte lowest_prio_voice = 0xff;

    // Serial.println("Getting voice");
    for (int i=0; i < VOICES_COUNT && activeVoicesCount < maxPerChannel; ++i) {
        if (i % 4 == 3) // Avoid noises channels for now
            continue;

        if (voice_properties[i].isAvailable() || !isOscActive(i)) {
            // Serial.print("Found avail voice ");
            // Serial.println(i);
            available_voice = i;
            continue;
        }

        // Serial.print("channel? ");
        // Serial.println(voice_properties[i].channel);

        if (voice_properties[i].channel != channel) {
            // We do not allow early recycling of voices belonging to other channels
            continue;
        }

        activeVoicesCount += 1;

        // Serial.print("recycling? ");
        // Serial.print(oscVolume(i));
        // Serial.print(" <? ");
        // Serial.print(lowest_volume);
        // Serial.print(" || ");
        // Serial.print(voice_properties[i].priority);
        // Serial.print(" < ");
        // Serial.print(lowest_prio);

        // No voice available
        // Determine which voice to drop, in order of priority:
        // - lowest volume
        // - oldest
        int volume = oscVolume(i);
        if ((volume < lowest_volume) ||
            (volume == lowest_volume && voice_properties[i].priority < lowest_prio)) {
            lowest_volume = volume;
            lowest_prio = voice_properties[i].priority;
            lowest_prio_voice = i;
            // Serial.print(" kept");
        }
        // Serial.println("");
    }

    if (activeVoicesCount < maxPerChannel && available_voice != 0xff) {
        // Reserve new available voice for our channel
        Serial.println("New voice");
        return available_voice;
    } else {
        // Early recycle existing voice.
        Serial.println("Recycling");
        return lowest_prio_voice;
    }
}

void setVoiceProperties(byte voice, byte channel, byte pitch)
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

void resetVoiceProperties()
{
    priorityCounter = 0;
    for (int i=0; i < VOICES_COUNT; ++i) {
        resetVoiceProperties(i);
    }
}

void updateVoiceProperties()
{
    for (int i=0; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel != 0xff && !isOscActive(i)){
            resetVoiceProperties(i);
        }
    }
}

byte findVoice(byte channel)
{
    for (int i=0; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel == channel) {
            return i;
        }
    }
    return 0xff;
}

byte findVoice(byte channel, byte pitch, byte start)
{
    for (int i=start; i < VOICES_COUNT; ++i) {
        if (voice_properties[i].channel == channel &&
            voice_properties[i].pitch == pitch) {
            return i;
        }
    }
    return 0xff;
}
