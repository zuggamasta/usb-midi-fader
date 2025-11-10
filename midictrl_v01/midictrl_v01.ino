#include <midi_serialization.h>
#include <usbmidi.h>

#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h>
HT16K33 display;

// modified USBMIDI example to only use the Midi CC part,

// See the README.md for the wiring diagram.

void sendCC(uint8_t channel, uint8_t control, uint8_t value) {
	USBMIDI.write(0xB0 | (channel & 0xf));
	USBMIDI.write(control & 0x7f);
	USBMIDI.write(value & 0x7f);
}

void sendNote(uint8_t channel, uint8_t note, uint8_t velocity) {
	USBMIDI.write((velocity != 0 ? 0x90 : 0x80) | (channel & 0xf));
	USBMIDI.write(note & 0x7f);
	USBMIDI.write(velocity &0x7f);
}

const int ANALOG_PIN_COUNT = 7;

// Change the order of the pins to change the ctrl or note order.
int analogPins[ANALOG_PIN_COUNT] = { A0, A1, A2, A3, A6, A7, A8 };

int ccValues[ANALOG_PIN_COUNT];

// Fill something into the ouputBuffer for the display
String displayOutput[] = {"S","P","r","k","0","0","0"};


int readCC(int pin) {
	// Convert from 10bit value to 7bit.
	return analogRead(pin) >> 3;
}

void setup() {
	// Initialize initial values.
	for (int i=0; i<ANALOG_PIN_COUNT; ++i) {
		pinMode(analogPins[i], INPUT);
		ccValues[i] = readCC(analogPins[i]);
	}

	Wire.begin();
	if (display.begin() == false){
    Serial.println("Device did not acknowledge! Freezing.");
		while(1);
	}
	display.setBrightness(1);
  Serial.println("Display acknowledged.");
}

void loop() {
	//Handle USB communication
	USBMIDI.poll();

	while (USBMIDI.available()) {
		// We must read entire available data, so in case we receive incoming
		// MIDI data, the host wouldn't get stuck.
		u8 b = USBMIDI.read();
	}

	for (int i=0; i<ANALOG_PIN_COUNT; ++i) {
		int value = readCC(analogPins[i]);

		// Send CC only if th has changed.
		if (ccValues[i] != value) {
			sendCC(0, i, value);

			// write the values the outputb buffer.
			// 127+ccValues[]*-1     <-- this inverts the values faders down min, faders up max
			// this will then be bit shifted down from 7-bit (0-127) to 4-bit (0-f) 
			// String(value, HEX) does convert a number in value to HEX string without the "0x" prefix so only "e" for 14
			displayOutput[i] = String((127+ccValues[i]*-1) >> 3 ,HEX);
			
			ccValues[i] = value;
		}
	}
	if (ccValues[3] > 63){
		display.print(displayOutput[0]+displayOutput[1]+displayOutput[2]+displayOutput[3]);
	}else{
		display.print(displayOutput[4]+displayOutput[5]+displayOutput[6]+displayOutput[3]);
	}
	display.updateDisplay();

	// Flush the displayOutput.
	USBMIDI.flush();
}
