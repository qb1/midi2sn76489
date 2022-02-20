#include <Arduino.h>

#include "config.h"
#include "board.h"
#include "control.h"

#include "MIDIUSB.h"
#define USE_TIMER_3     true
#include "TimerInterrupt.h"

void startTimers();

void setup()
{
	setupBoard();
    setupControl();

	startTimers();

	selectChip(0);
}

void loop() {
	midiEventPacket_t rx = MidiUSB.read();
	switch (rx.header) {
		case 0:
			break; //No pending events
			
		case 0x9:
			noteOn(
				rx.byte1 & 0xF,  //channel
				rx.byte2,        //pitch
				rx.byte3         //velocity
			);
			break;
			
		case 0x8:
			noteOff(
				rx.byte1 & 0xF,  //channel
				rx.byte2,        //pitch
				rx.byte3         //velocity
			);
			break;
			
		case 0xB:
			controlChange(
				rx.byte1 & 0xF,  //channel
				rx.byte2,        //control
				rx.byte3         //value
			);
			break;
			
		default:
			Serial.print("Unhandled MIDI message: ");
			Serial.print(rx.header, HEX);
			Serial.print("-");
			Serial.print(rx.byte1, HEX);
			Serial.print("-");
			Serial.print(rx.byte2, HEX);
			Serial.print("-");
			Serial.println(rx.byte3, HEX);
	}
}

void TimerHandler()
{
	updateChannels();
}

void startTimers()
{
	ITimer3.init();

	if (ITimer3.attachInterruptInterval(REFRESH_RATE, TimerHandler, 0))
	{
		Serial.print(F("Starting  ITimer3 OK, millis() = ")); Serial.println(millis());
	}
	else {
		Serial.println(F("Can't set ITimer3. Select another freq. or timer"));
    }
}
