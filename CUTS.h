#pragma once
#pragma once
#include "common.h"
#include "computerInterface.h"
#include <IntervalTimer.h>

extern ComputerInterface *computerInterface; //We sadly need to have 

class CUTS : public virtual ComputerInterface {
private:
	typedef enum frequencies {
		lowFreq = 0,
		highFreq = 1,
		transitionFreq,
		unknownFreq,
	} frequency;
	typedef struct {
		uint8_t cyclesInLowFreqSymbol = 0;
		uint8_t cyclesInHighFreqSymbol = 0;

		/*Carrier frequency periods in microseconds*/
		/*We actually count both edges, so divide by two.*/
		int LowFreqPeriod = 0;
		int HighFreqPeriod = 0;
		int window = 0; /*Tolerance, must be within +- window to count */
	} CutsSettings;

	bool inByte = false;

	inline void underLengthInterval(unsigned long interval);
	inline void overLengthInterval(unsigned long interval);
	inline void unknownInterval(unsigned long interval);
	inline void recordBit(frequency freq);
	inline void registerNote(frequency freq);

	IntervalTimer txTimer;
	bool timerRunning = false; // Because IntervalTimer is too mean to let us see its status.

	class ISRhelper_t {
	public:
		static void outputISR();
		static void inputISR();
	} ISRhelper;

	frequency nextBit(); // Return the frequency value corresponding to the next thing we need to send
	void stopTxTimer();
	bool nextByte(); // Update the byte buffer. Returns the buffer state (true if more data ready, false if there's nothing in the queue and we should check the end state).

	byte currentByte;
	bool dataToSend = false;
	byte bufferByte;
	bool endRequested = false;

public:
	CUTS(pin inputPin, pin outputPin);

	////Called by the ISR
	void recordChange(void);
	void toggleOutput(void);

	////Indicates that a byte has been received and is ready to be processed.
	//bool newByteAvaliable = false;
	////The value of said received byte.
	//byte data;


	////Indicates that there is space in the transmit buffer.
	//bool bufferAvaliable = false;

	////Enter a byte into the transmit queue
	////Returns true if ok, false if unable to send for whatever reason
	bool sendByte(byte b);

	////Inform the interface that the file being sent has ended. Ideally this will also be detected with a timeout, but why rely on that?
	void endTransmission();
	//
	//CUTS related settings, must be set by the user, preferably in the constructor of an inheriting class.
	CutsSettings settings;
};
