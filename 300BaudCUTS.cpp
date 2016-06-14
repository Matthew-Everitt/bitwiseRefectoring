#include "300BaudCUTS.h"

threeHundredBaudCUTS::threeHundredBaudCUTS(pin inputPin, pin outputPin) : CUTS(inputPin, outputPin) {
	settings.cyclesInLowFreqSymbol = 8;
	settings.cyclesInHighFreqSymbol = 16;

	/*Carrier frequency periods in microseconds*/
	/*We actually count both edges, so divide by two.*/
	settings.LowFreqPeriod = (int)((1e6 / 1200.0) / 2.0);
	settings.HighFreqPeriod = (int)((1e6 / 2400.0) / 2.0);
	settings.window = 100; /*Tolerance, must be within +- window to count */

}
