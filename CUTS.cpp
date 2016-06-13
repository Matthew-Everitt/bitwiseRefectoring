#include "CUTS.h"
inline void CUTS::underLengthInterval(unsigned long interval) {
#ifdef reportBadPeriods
	Serial.print("Underlength interval of ");
	Serial.print(interval);
	Serial.println(" microseconds found");
#endif
}

inline void CUTS::overLengthInterval(unsigned long interval) {
#ifdef reportBadPeriods

	Serial.print("Overlength interval of ");
	Serial.print(interval);
	Serial.println(" microseconds found");
#endif
}

inline void CUTS::unknownInterval(unsigned long interval) {
#ifdef reportBadPeriods

	Serial.print("Unknown interval of ");
	Serial.print(interval);
	Serial.println(" microseconds found");
#endif
}

inline void CUTS::recordBit(frequency freq) {
	static bool startBit = false;
	static uint8_t count = 0;
	static byte data = 0;
#ifdef reportRawBits
	Serial.print("Bit :  ");
	Serial.println(freq);
#endif
	if (!startBit && freq == 0) {
		startBit = true;

	} else if (startBit && count < 8) {
		data |= (freq << count);
		count++;

	} else if (count == 8 && freq == 1) {
		/*Stop bit found, record and reset*/
		//     Serial.println("Setting new byte");
		this->data = data;
		this->newByteAvaliable = true;

		startBit = false;
		count = 0;
		data = 0;
		inByte = false;

	} else {
		Serial.print("-");
	}
}


inline void CUTS::registerNote(frequency freq) {
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
	} else {
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
		if (lowFreq == freq) target = this->settings.cyclesInLowFreqSymbol;
		else target = this->settings.cyclesInHighFreqSymbol;
	}
	//Serial.println((uint8_t)freq);
}

void CUTS::recordChange(void) {
	digitalWrite(outputPin, HIGH);
	//carrierTimer.restart();
	//carrierPresent = true;

	//Serial.print("- ");
	static unsigned long previous = 0;
	unsigned long current = micros();
	int interval = (current - previous);


	/*I hate if ladders, but it does seem to be the right thing here*/
	if (settings.HighFreqPeriod - settings.window > interval)                                    	underLengthInterval(interval);
	else if (settings.HighFreqPeriod - settings.window <= interval && interval < settings.HighFreqPeriod + settings.window) 	registerNote(highFreq);
	else if (settings.HighFreqPeriod + settings.window <= interval && interval < settings.LowFreqPeriod - settings.window) 	registerNote(transitionFreq);
	else if (settings.LowFreqPeriod - settings.window <= interval && interval < settings.LowFreqPeriod + settings.window) 	registerNote(lowFreq);
	else if (settings.LowFreqPeriod + settings.window < interval)                                    	overLengthInterval(interval);
	else unknownInterval(interval);

	previous = current;
}

bool CUTS::sendByte(byte b) {
	return false;
}

void CUTS::endTransmission() {}


//void carrierLost(void) {
//	/* The timer has expired which means we've not had an interrupt recently. This is probably a good time to flush data out etc*/
//	//carrierTimer.stop();
//	//carrierPresent = false;
//#ifdef reportCarrierState
//	Serial.println("Carrier lost");
//#endif
//}

