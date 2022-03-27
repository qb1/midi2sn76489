#include <Arduino.h>

#include "board.h"
#include "config.h"

// #define ENABLE_DEBUG_LOGS
#include "logs.h"

static const int clockOutputPin = 9;   // OC1A output pin for ATmega32u4 (Arduino Micro)
static const int clockStepper = 3;     // Clock divider from 16 MHz: 1=8MHz, 2=4...
static const int pinWE = 8;

const byte REG_TONE1 = 0b00000000;
const byte REG_TONE2 = 0b00100000;
const byte REG_TONE3 = 0b01000000;
const byte REG_NOISE = 0b01100000;
const byte REG_FREQ  = 0b00000000;
const byte REG_CTRL  = 0b00000000;
const byte REG_ATT   = 0b00010000;

void writeByte(byte value);
byte buildChipChannel(byte chip_channel);
void setupSNChips();
void setupClock();

void lockChip(byte chip)
{
    if (chip > CHIP_COUNT) {
        ERROR_MSG("Trying to address non-existing chip!", chip);
        return;
    }

    // Disable interrupts otherwise timers might select a different chip.
    noInterrupts();
    digitalWrite(A0 + chip, LOW);
}

void unlockChip(byte chip)
{
    digitalWrite(A0 + chip, HIGH);
    interrupts();
}

void updateVolume(byte chip, byte chip_channel, byte value)
{
	value = 0xf - (value & 0xf);

	byte to_write = value;                      // Attenuation value
	to_write |= buildChipChannel(chip_channel); // Select chip's channel & select bit
	to_write |= 1 << 4;                         // Attenuation function

    lockChip(chip);
	writeByte(to_write);
    unlockChip(chip);
}

void updateFreq(byte chip, byte chip_channel, unsigned int value)
{
	value &= 0b1111111111;

	// Freq function is bit 0 on R2
	byte to_write1 = value & 0b1111;
	to_write1 |= buildChipChannel(chip_channel);
	byte to_write2 = value >> 4;

    lockChip(chip);
	writeByte(to_write1);
	writeByte(to_write2);
    unlockChip(chip);
}

void updateNoise(byte chip, byte value)
{
	byte chip_channel = 3;
	value &= 0b111;

	// Noise control is bit 0 on R2
	byte to_write = value;                       // Attenuation value
	to_write |= buildChipChannel(chip_channel);  // Select chip's channel & select bit

    lockChip(chip);
	writeByte(to_write);
    unlockChip(chip);
}

void setupBoard()
{
    INFO_MSG("Setting up board.");
	setupSNChips();
	setupClock();

	// Chips start with random noises - must mute them up ASAP.
	muteAll();
    INFO_MSG("Board setup done.");
}

void muteAll()
{
    INFO_MSG("Muting all voices.");
	for (int i=0; i < CHIP_COUNT; ++i) {
        for (int j=0; j < VOICES_PER_CHIP; ++j) {
		    updateVolume(i, j, 0);
        }
	}
}

void writeByte(byte value)
{
	// Setup data bus
	for (int i=0; i < 2; ++i) {
		digitalWrite(14 + i, (value >> i) & 1); // lower bits are on 10 & 11
	}
	for (int i=2; i < 8; ++i) {
		digitalWrite(i, (value >> i) & 1); // upper on 2-7
	}

	// Strobe write enable
	digitalWrite(pinWE, LOW);
	delayMicroseconds(10); // Ignore READY... sad.
	digitalWrite(pinWE, HIGH);
}

byte buildChipChannel(byte chip_channel)
{
	return 0b10000000 | (chip_channel << 5);
}

void setupSNChips()
{
	// Data output pins, cannot use PINs 0 & 1 (tx & rx)
	for(int i=0; i < 2; ++i) {
		pinMode(14 + i, OUTPUT);
		digitalWrite(i, LOW);
	}
	for(int i=2; i < 8; ++i) {
		pinMode(i, OUTPUT);
		digitalWrite(i, LOW);
	}

	pinMode(pinWE, OUTPUT);
	digitalWrite(pinWE, HIGH); // Inverted logic

    for (int i=0; i < 4; ++i) {
        // Analog pins used as CE
        pinMode(A0 + i, OUTPUT);
        // Make sure no chip is selected: expected state through the program
		digitalWrite(A0 + i, HIGH); // Inverted logic
	}
}

void setupClock()
{
	// Setup the clock to drive the SN76489
	pinMode(clockOutputPin, OUTPUT);

	// Set Timer 1 CTC mode with no prescaling.  OC1A toggles on compare match
	//
	// WGM12:0 = 010: CTC Mode, toggle OC
	// WGM2 bits 1 and 0 are in TCCR1A,
	// WGM2 bit 2 and 3 are in TCCR1B
	// COM1A0 sets OC1A (arduino pin 9 on Arduino Micro) to toggle on compare match
	TCCR1A = ( (1 << COM1A0));

	// Set Timer 1  No prescaling  (i.e. prescale division = 1)
	TCCR1B = ((1 << WGM12) | (1 << CS10));
	// Make sure Compare-match register A interrupt for timer1 is disabled
	TIMSK1 = 0;
	// This value determines the output frequency
	OCR1A = clockStepper;

	// Wait a bit for chip ready
	delay(100);
}
