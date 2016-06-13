#pragma once
#pragma once
#include "common.h"
#include "computerInterface.h"
class CUTS : public virtual computerInterface {
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


public:
	//Called by the ISR
	void recordChange(void);

	//Indicates that a byte has been received and is ready to be processed.
	bool newByteAvaliable = false;
	//The value of said received byte.
	byte data;


	//Indicates that there is space in the transmit buffer.
	bool bufferAvaliable = false;

	//Enter a byte into the transmit queue
	//Returns true if ok, false if unable to send for whatever reason
	bool sendByte(byte b);

	//Inform the interface that the file being sent has ended. Ideally this will also be detected with a timeout, but why rely on that?
	void endTransmission();
	
	//CUTS related settings, must be set by the user, preferably in the constructor of an inheriting class.
	CutsSettings settings;
};
