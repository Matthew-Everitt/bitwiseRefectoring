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
	void recordChange(void);
	bool newByteAvaliable = false;;
	bool sending = false;;
	byte data;
	CutsSettings settings;
};
