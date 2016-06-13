#include <TimerOne.h>

#include "common.h"

#include "FileTools.h"

#include "computerInterface.h"
#include "300BaudCUTS.h"

#include "storageInterface.h"
#include "namedFile.h"


#include <SdFat.h>
#include <SdSpi.h>

SdFat SD;
File sdFile;

//Things it might be useful to extern in other files
settings_t settings;
bool SdPresent = false;



//bool carrierPresent = false;


//bool newByte = false;
//byte recievedByte = 0;



threeHundredBaudCUTS thb;
computerInterface *cuts= &thb;

namedFile named;
storageInterface * f = &named;

void cutsInputPinISR(void) {
	cuts->recordChange();
}
//void cutsCarrierLostISR(void) {
//	cuts.carrierLost();
//}

void updateCUTS(void){
	if (cuts->newByte) {
#ifdef rawBytes
		Serial.print("rawByte,");
		Serial.println(recievedByte, HEX);
#endif
		f->RX(cuts->data);
		cuts->newByte = false;
	}
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
	} else {
		Serial.println("SD card initialization failed! SD will not be available");
		SdPresent = false;
	}

	pinMode(inputPin, INPUT);
	pinMode(outputPin, OUTPUT);
	attachInterrupt(inputPin, cutsInputPinISR, CHANGE);

	///*Use carrierTimer to detect the absence of a carrier, which would allow us to go into sleep mode, or whatever*/
	//carrierTimer.initialize(833 * 50); /*Delay in microseconds - 833 microseconds is the longest period (1/1200Hz), so 50 times that is a reasonable number of events not happening*/
	//carrierTimer.attachInterrupt(cutsCarrierLostISR);
	////carrierTimer.start();

}



void loop() {
	updateCUTS();

}
