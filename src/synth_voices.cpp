#include <Arduino.h>

#include "config.h"
#include "synth_oscs.h"
#include "synth_voices.h"
#include "synth_internals.h"

// #define ENABLE_DEBUG_LOGS
#include "logs.h"

VoiceProperties voice_properties[VOICES_COUNT];

static uint32_t priorityCounter = 0;

void setupVoiceProperties()
{
    priorityCounter = 0;
    for (int i=0; i < VOICES_COUNT; ++i) {
        resetVoiceProperties(i);
    }
}

byte findAvailableVoice(int channel, int onChip, int maxPerChannel, SynthChannel::Type type)
{
    int activeVoicesCount = 0;

    // Will store first found fully available voice.
    // Will be selected if there are no more than maxPerChannel voices used already.
    byte available_voice = 0xff;

    // While exploring, also keep track of track we could recycle early if necessary.
    uint32_t lowest_prio = -1;
    int lowest_volume = 0xff;
    bool found_releasing_voice = false;
    byte recycling_voice = 0xff;

    DEBUG_MSG("---> Getting voice for channel ", channel);
    for (int i=0; i < VOICES_COUNT && activeVoicesCount < maxPerChannel; ++i) {
        if ((onChip & (1 << (i / 4))) == 0) {
            continue;
        }
        if ((type == SynthChannel::Music && (i % 4 == 3 || i % 4 == 2)) ||
            (type == SynthChannel::Drum && i % 4 != 3))
            continue;

        if (voice_properties[i].isAvailable() || !isOscActive(i)) {
            DEBUG_MSG("Found avail voice ", i);
            available_voice = i;
            continue;
        }

        if (voice_properties[i].channel != channel) {
            // We do not allow early recycling of voices belonging to other channels that are still in use
            DEBUG_MSG("Skipping voice from channel ", voice_properties[i].channel);
            continue;
        }

        activeVoicesCount += 1;

        DEBUG_MSG("Might recycle: ", oscTargetVolume(i), " <? ", lowest_volume, " || ",
                  voice_properties[i].priority, " < ", lowest_prio);

        // No voice available
        // Determine which voice to drop, in order of priority:
        // - being released
        // - lowest volume
        // - oldest
        int volume = oscTargetVolume(i);
        if ((!found_releasing_voice && isOscReleasing(i)) ||
            (volume < lowest_volume) ||
            (volume == lowest_volume && voice_properties[i].priority < lowest_prio)) {
            if (found_releasing_voice && !isOscReleasing(i)) {
                // Once we found a releasing voice, do not recycle a non-release
                continue;
            }
            found_releasing_voice && isOscReleasing(i);
            lowest_volume = volume;
            lowest_prio = voice_properties[i].priority;
            recycling_voice = i;
            // DEBUG_MSG("-> ... kept ", i);
        }
    }

    if (activeVoicesCount < maxPerChannel && available_voice != 0xff) {
        // Reserve new available voice for our channel
        DEBUG_MSG("---< ", activeVoicesCount, " actives - using new: ", available_voice);
        return available_voice;
    } else {
        // Early recycle existing voice.
        DEBUG_MSG("---< ", activeVoicesCount, " actives - recycling: ", recycling_voice);
        return recycling_voice;
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
