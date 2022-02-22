#pragma once
#include <Arduino.h>

/*
 * Define low-level function to handle the board:
 *  - setup
 *  - selecting SN chips
 *  - changing SN chips config
 */

// Call this once at setup
void setupBoard();

// Save / restore chip selection state

// All functions act on selected chip.
void muteAll();
void updateVolume(byte chip, byte chip_channel, byte value);
void updateFreq(byte chip, byte chip_channel, unsigned int value);
void updateNoise(byte chip, byte value);
