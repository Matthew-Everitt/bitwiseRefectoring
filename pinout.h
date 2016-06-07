#pragma once
enum pins {
	//LCD display pins
	//TODO - Check these, I think DC & RST are swapped
	dispCLK = 18,
	dispMOSI = 19,
	dispSCE = 22,
	dispRST = 20,
	dispDC = 21,
	dispLED = 23,

	//Rotary encoder pins
	encPin1 = 9,
	encPin2 = 10,
	encButt = 11,

	//Simple buttons
	playBut = 3, //Not used?
	backBut = 2,

	//CUTS interface pins
	inputPin = 15,
	outputPin = 13,

	//SD card pins
	chipSelect = 6,
	sdMOSI = 7,
	sdMISO = 12,
	sdSCK = 14,

  LED = 13,
};

