#include <TimerOne.h>

#include "common.h"
#include "namedFile.h"
#include "FileTools.h"
#include <SdFat.h>
#include <SdSpi.h>

SdFat SD;
File sdFile;

//Things it might be useful to extern in other files
settings_t settings;
bool SdPresent = false;



bool carrierPresent = false;
bool inByte = false;

bool newByte = false;
byte recievedByte = 0;

class threeHundredBaudCUTS {
public:
	inline void underLengthInterval(unsigned long interval) {
#ifdef reportBadPeriods
		Serial.print("Underlength interval of ");
		Serial.print(interval);
		Serial.println(" microseconds found");
#endif
	}

	inline void overLengthInterval(unsigned long interval) {
#ifdef reportBadPeriods

		Serial.print("Overlength interval of ");
		Serial.print(interval);
		Serial.println(" microseconds found");
#endif
	}

	inline void unknownInterval(unsigned long interval) {
#ifdef reportBadPeriods

		Serial.print("Unknown interval of ");
		Serial.print(interval);
		Serial.println(" microseconds found");
#endif
	}

	inline void recordBit(frequency freq) {
		static bool startBit = false;
		static uint8_t count = 0;
		static byte data = 0;
#ifdef reportRawBits
		Serial.print("Bit :  ");
		Serial.println(freq);
#endif
		if (!startBit && freq == 0) {
			startBit = true;

		}
		else if (startBit && count < 8) {
			data |= (freq << count);
			count++;

		}
		else if (count == 8 && freq == 1) {
			/*Stop bit found, record and reset*/
			//     Serial.println("Setting new byte");
			recievedByte = data;
			newByte = true;

			startBit = false;
			count = 0;
			data = 0;
			inByte = false;

		}
		else {
			Serial.print("-");
		}
	}


	inline void registerNote(frequency freq) {
		static frequency previousFreq = unknownFreq;
		static uint8_t count = 0;

		static uint8_t target = 0;

		/*Bytes always start with a 0 (lowFreq), while the non-data carrier is a stream of ones. */
		if (!inByte  &&  lowFreq != freq) return;

		inByte = true;

		if (freq == previousFreq) {
			count++;
			if (count == target) {
				recordBit(freq);
				count = 0;
			}
		}
		else {
#ifdef reportFreqChanges
			if (count != 0) {
				Serial.print("Freq change from ");
				Serial.print(previousFreq);
				Serial.print(" to ");
				Serial.print(freq);
				Serial.print(" after ");
				Serial.print(count / 2.0);
				Serial.println(" cycles.");
			}
#endif
			count = 1;
			previousFreq = freq;
			if (lowFreq == freq) target = 8;
			else target = 16;
		}
		//Serial.println((uint8_t)freq);
	}


	void recordChange(void) {
		digitalWrite(outputPin, HIGH);
		carrierTimer.restart();
		carrierPresent = true;

		//Serial.print("- ");
		static unsigned long previous = 0;
		unsigned long current = micros();
		int interval = (current - previous);

		/*We actually count both edges, so use 0.5 rather than one. The reason for this is to allow counting half bits*/
#define longPeriod  ((int)(0.5e6/1200.0))  /*Period of the low frequency in microseconds  */
#define shortPeriod ((int)(0.5e6/2400.0))  /*Period of the high frequency in microseconds */
#define window      ((int)100.0)         /*Tolerence, must be within +- window to count */

	/*I hate if ladders, but it does seem to be the right thing here*/
		if (shortPeriod - window > interval)                                    	underLengthInterval(interval);
		else if (shortPeriod - window <= interval && interval < shortPeriod + window) 	registerNote(highFreq);
		else if (shortPeriod + window <= interval && interval < longPeriod - window) 	registerNote(transitionFreq);
		else if (longPeriod - window <= interval && interval < longPeriod + window) 	registerNote(lowFreq);
		else if (longPeriod + window < interval)                                    	overLengthInterval(interval);
		else unknownInterval(interval);

		previous = current;
	}

	void carrierLost(void) {
		/* The timer has expired which means we've not had an intertupt recently. This is probably a good time to flush data out etc*/
		carrierTimer.stop();
		carrierPresent = false;
#ifdef reportCarrierState
		Serial.println("Carrier lost");
#endif
	}
};


threeHundredBaudCUTS cuts;

void cutsInputPinISR(void) {
	cuts.recordChange();
}
void cutsCarrierLostISR(void) {
	cuts.carrierLost();
}


void setup() {
	Serial.begin(115200);
	while (!Serial);

	//SdFat library only supports this if SD_SPI_CONFIGURATION is set to 1 in SdFatConfig.h
	SPI.setMOSI(sdMOSI);
	SPI.setMISO(sdMISO);
	SPI.setSCK(sdSCK);

	settings.overwrite = false;
	settings.detokenise = true;


	if (SD.begin(chipSelect)) {
		SdPresent = true;
	}
	else {
		Serial.println("SD card initialization failed! SD will not be available");
		SdPresent = false;
	}

	pinMode(inputPin, INPUT);
	pinMode(outputPin, OUTPUT);
	attachInterrupt(inputPin, cutsInputPinISR, CHANGE);

	/*Use carrierTimer to detect the absence of a carrier, which would allow us to go into sleep mode, or whatever*/
	carrierTimer.initialize(833 * 50); /*Delay in microseconds - 833 microseconds is the longest period (1/1200Hz), so 50 times that is a reasonable number of events not happening*/
	carrierTimer.attachInterrupt(cutsCarrierLostISR);
	//carrierTimer.start();
}


namedFile named;
format * f = &named;
void loop() {
	digitalWrite(outputPin, LOW);
	if (newByte) {
#ifdef rawBytes
		Serial.print("rawByte,");
		Serial.println(recievedByte, HEX);
#endif
		f->RX(recievedByte, "");

		newByte = false;

	}

}
