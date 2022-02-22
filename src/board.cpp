#include <Arduino.h>

#include "board.h"

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

void selectAllChips(bool select)
{
	for (int i=0; i < 4; ++i) {
		digitalWrite(A0 + i, select ? LOW : HIGH);
	}
}
void selectChip(byte chip)
{
	chip = chip % 4;
	selectAllChips(false);
	digitalWrite(A0 + chip, LOW);
}

void updateVolume(byte chip_channel, byte value)
{
	value = 0xf - (value & 0xf);

	byte to_write = value;                      // Attenuation value
	to_write |= buildChipChannel(chip_channel); // Select chip's channel & select bit
	to_write |= 1 << 4;                         // Attenuation function
	writeByte(to_write);
}

void updateFreq(byte chip_channel, unsigned int value)
{
	value &= 0b1111111111;

	byte to_write = value & 0b1111;
	to_write |= buildChipChannel(chip_channel);
	// Freq function is bit 0 on R2
	writeByte(to_write);
	to_write = value >> 4;
	writeByte(to_write);
}

void updateNoise(byte value)
{
	byte chip_channel = 3;
	value &= 0b111;

	byte to_write = value;                       // Attenuation value
	to_write |= buildChipChannel(chip_channel);  // Select chip's channel & select bit
	// Noise control is bit 0 on R2
	writeByte(to_write);
}

void setupBoard()
{
	setupSNChips();
	setupClock();

	// Chips start with random noises - must mute them up ASAP.
	selectAllChips(true);
	muteAll();
    selectAllChips(false);
}

void muteAll()
{
	for (int i=0; i < 4; ++i) {
		updateVolume(i, 0);
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
	delayMicroseconds(100); // Ignore READY... sad.
	digitalWrite(pinWE, HIGH);
}

byte buildChipChannel(byte chip_channel)
{
	return 0b10000000 | (chip_channel << 5);
}

void setupSNChips()
{
	// Everything as out
	for(int i=0; i < 2; ++i) {
		pinMode(14 + i, OUTPUT);
		digitalWrite(i, LOW);
	}
	for(int i=2; i < 8; ++i) {
		pinMode(i, OUTPUT);
		digitalWrite(i, LOW);
	}

	pinMode(pinWE, OUTPUT);

	// Analog pins used as CE
	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);

	digitalWrite(pinWE, HIGH);
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
