/*
 * Define necessary definitions for handling MIDI inputs
 *  - notes to clock dividers
 *  - names
 *  - ...
 */
#include <stdint.h>

const unsigned int NOTES[128] = {3820, 3609, 3404, 3215, 3034, 2864, 2703, 2551, 2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607, 1517, 1432, 1352, 1276, 1204, 1136, 1072, 1012, 956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 225, 213, 201, 190, 179, 169, 159, 150, 142, 134, 127, 119, 113, 106, 100, 95, 89, 84, 80, 75, 71, 67, 63, 60, 56, 53, 50, 47, 45, 42, 40, 38, 36, 34, 32, 30, 28, 27, 25, 24, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 13, 12, 11, 11, 10, 9, 9, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2};

// Notes below are not accessible, >10bits of division
const unsigned char FIRST_NOTE = 24;

// Above, it's probably going to be really out of tune because of integer division
const unsigned char LAST_NOTE = 127;

const char* pitch_name(uint8_t pitch) {
	static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	return names[pitch % 12];
}

int pitch_octave(uint8_t pitch) {
	return (pitch / 12) - 1;
}