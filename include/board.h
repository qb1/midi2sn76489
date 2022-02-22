#include <Arduino.h>

/*
 * Define low-level function to handle the board:
 *  - setup
 *  - selecting SN chips
 *  - changing SN chips config
 */

// Call this once at setup
void setupBoard();

// Select / deselect all chips if parameter true / false
// Note: this is not inverted logic (as the CE pins are)
void selectAllChips(bool select);

// Select specified chip, unselect every other.
void selectChip(byte chip);

// All functions act on selected chip, if any.
void muteAll();
void updateVolume(byte chip_channel, byte value);
void updateFreq(byte chip_channel, unsigned int value);
void updateNoise(byte value);
