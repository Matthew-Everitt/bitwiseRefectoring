#pragma once
#include "common.h"
class threeHundredBaudCUTS {
private:

	typedef enum frequencies {
		lowFreq = 0,
		highFreq = 1,
		transitionFreq,
		unknownFreq,
	} frequency;

	bool inByte = false;

	inline void underLengthInterval(unsigned long interval);
	inline void overLengthInterval(unsigned long interval);
	inline void unknownInterval(unsigned long interval);
	inline void recordBit(frequency freq);
	inline void registerNote(frequency freq);


public:
	void recordChange(void);
	bool newByte;
	byte data;
};
