#include <Arduino.h>

#include "config.h"
#include "board.h"
#include "synth.h"

#include "MIDIUSB.h"
#define USE_TIMER_3     true
#include "TimerInterrupt.h"

//#define ENABLE_DEBUG_LOGS
#include "logs.h"

void startTimers();

void setup()
{
	setupBoard();
    setupSynth();

	startTimers();
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

		case 0xE:
			bendChange(
				rx.byte1 & 0xF,  //channel
                // we ignore byte 2, on my controller it seems to = msb of byte 3
				rx.byte3         //value
			);
			break;

        case 0xF:
            // MIDI clock signal
            if (rx.byte1 == 0xF8) {
                static unsigned long last = 0;
                static unsigned long bpm = 0;
                unsigned long current = micros();
                if (last != 0) {
                    unsigned long elapsed = current - last;
                    unsigned long new_bpm = 2500000 / elapsed;
                    if (new_bpm != bpm) {
                        bpm = new_bpm;
                        Serial.print("New BPM: ");
			            Serial.println(bpm);
                        Serial.println(elapsed);
                    }
                }
                last = current;
            }

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
	updateSynth();
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
