#include "CUTS.h"
#include "debugLevels.h"

inline void CUTS::underLengthInterval(unsigned long interval) {
#ifdef reportBadPeriods
	Serial.print("Under length interval of ");
	Serial.print(interval);
	Serial.println(" microseconds found");
#endif
}

inline void CUTS::overLengthInterval(unsigned long interval) {
#ifdef reportBadPeriods

	Serial.print("Over length interval of ");
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
#ifdef reportNewBytes
		Serial.print("CUTS has found a byte :  0x"); Serial.println(this->data, HEX);
#endif
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

CUTS::CUTS(pin inputPin, pin outputPin) {
	pinMode(inputPin, INPUT);
	pinMode(outputPin, OUTPUT);
	attachInterrupt(inputPin, this->ISRhelper.inputISR, CHANGE);
}

void CUTS::recordChange(void) {
	digitalWrite(outputPin, HIGH);
	//carrierTimer.restart();
	//carrierPresent = true;

	//Serial.print("- ");
	static unsigned long previous = 0;
	unsigned long current = micros();
	int interval = (current - previous);

#ifdef reportInputStateChange
	Serial.print("Input pin changed after "); Serial.print(interval); Serial.println("us");
#endif

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
	bool ret = false;



	if (!dataToSend) {
		currentByte = b;
		dataToSend = true;
		ret= true;
	}

	if (bufferAvaliable) {
		Serial.print("Adding 0x"); Serial.println(b);
		bufferByte = b;
		bufferAvaliable = false;
		ret= true;
	} else {
		Serial.println("Buffer full");
		ret = false;
	}
	if (!(this->timerRunning)) {
		Serial.println("Starting timer");
		this->timerRunning = true;
		txTimer.begin(this->ISRhelper.outputISR, this->settings.HighFreqPeriod);
	}

	return ret;
}

void CUTS::endTransmission() {
	this->endRequested = true;
}

void CUTS::toggleOutput() {
	static bool dataCrossing = false;
	static byte cyclesDone = 0;
	static byte cyclesNeeded = 4;
	static bool toggleOnDataCrossings;

	if (cyclesDone == cyclesNeeded) {
		Serial.println("Next");
		switch (nextBit()) {
		case highFreq:
			//cyclesNeeded = this->settings.cyclesInHighFreqSymbol/2;
			toggleOnDataCrossings = true;
			break;
		case lowFreq:
			//cyclesNeeded = this->settings.cyclesInLowFreqSymbol;
			toggleOnDataCrossings = false;
			break;
		}
		cyclesDone = 0;
	}

	if (!dataCrossing || toggleOnDataCrossings) {
		Serial.println("toggle");
		digitalWriteFast(outputPin, !digitalReadFast(outputPin));
	} else {
		Serial.println("Don't toggle");
	}
	if (dataCrossing) cyclesDone++;
	dataCrossing = !dataCrossing;
}

//void carrierLost(void) {
//	/* The timer has expired which means we've not had an interrupt recently. This is probably a good time to flush data out etc*/
//	//carrierTimer.stop();
//	//carrierPresent = false;
//#ifdef reportCarrierState
//	Serial.println("Carrier lost");
//#endif
//}

CUTS::frequency CUTS::nextBit() {
	static byte bitsSent = 0;
	static bool startBitSent = false;
	static bool stopBitSent = false;
	Serial.print("Next bit is ");
	if (!this->dataToSend) { //We have no data, so send just the carrier.
		Serial.println("Carrier");
		return highFreq;
	} else if (!startBitSent) {
		Serial.println("Start");
		startBitSent = true;
		return lowFreq;
	} else if (bitsSent < 8) {
		//Send next bit;
		bool value = (currentByte >> bitsSent) & 1;
		Serial.print("bit "); Serial.print(bitsSent); Serial.print(" of 0b"); Serial.print(currentByte, BIN); Serial.print(" ("); Serial.print(value); Serial.println(")");
		bitsSent++;
		return (frequency)value;

	} else { //Send stop bit and reset for the next byte
		if (!nextByte()) { //We need to call nextByte to move onto the next byte. If it returns false we've run out of data and need to question if we should stop.
			if (endRequested) {
				if (!stopBitSent) { //We need to end, but we've not sent a stop bit, so we should, you know, do that.
					stopBitSent = true; 
					Serial.println("Stop");
					this->dataToSend = true;
					return highFreq;
				} else {
					//Reset for next byte
					startBitSent = false;
					bitsSent = 0;
					stopBitSent = false;
					stopTxTimer();
					return highFreq; //Not that it matters
				}
			} else {
				//No next byte to send so pump out the carrier
				return highFreq;
			}
		} else {
			//Reset for next byte
			startBitSent = false;
			bitsSent = 0;
			//Send the stop bit for the previous byte
			return highFreq;
		}
	}
}
void CUTS::stopTxTimer() {
	Serial.println("End");

	txTimer.end();
	this->timerRunning = false;

	endRequested = false;
	digitalWrite(outputPin, HIGH);
}
bool CUTS::nextByte() {
	Serial.print("NextByte :  ");
	if (bufferAvaliable) {
		dataToSend = false;
		Serial.println("Carrier");
		return false;
	} else {
		Serial.print("0x"); Serial.println(bufferByte);
		currentByte = bufferByte;
		dataToSend = true;
		bufferAvaliable = true;
		return true;
	}
}

void CUTS::ISRhelper_t::outputISR() {
	computerInterface->toggleOutput();
}


void CUTS::ISRhelper_t::inputISR() {
	computerInterface->recordChange();
}