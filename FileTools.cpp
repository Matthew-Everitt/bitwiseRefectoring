#include "FileTools.h"
#include "systemState.h"
#include <SPI.h>


#define SEP_CHAR '-' //The character used to seperate the filename from the number we add to avoid conflict.
//namespace fileTools{

void setupSD() {
	//SdFat library only supports this if SD_SPI_CONFIGURATION is set to 1 in SdFatConfig.h
	SPI.setMOSI(sdMOSI);
	SPI.setMISO(sdMISO);
	SPI.setSCK(sdSCK);

	if (SD.begin(chipSelect)) {
		systemState.sdPresent = true;
		systemState.freeSpace = freeSpace();
	} else {
		Serial.println("SD card initialization failed! SD will not be avaliable");
		systemState.sdPresent = false;
	}
}


int bytesToString(float bytes, char * buffer, int n) {
	char * prefixes = " kMGT";
	int count = 0;
	while (bytes >= 1.0) {
		bytes /= 1024.0;
		count++;
	}
	bytes *= 1024.0;
	count--;
	return snprintf(buffer, n, "%6.2f%cB", bytes, prefixes[count]);
}

float freeSpace() {
	return 512.0 * SD.vol()->freeClusterCount() * SD.vol()->blocksPerCluster();
}
//
//byte longNameToShortName(char * longName, char * out) {
//	//longName must be a sensible C string ( with a \0 terminator ). There is a limit of 100 chars (to prevent us reading forever if there are no zeros)
//	char * p = longName;
//	char * dot;
//	byte i = 0;
//	byte nameLength = 0;
//	bool tooLong = false;
//
//	name_t name;
//
//	//Find the last '.', giving us the extension
//	while (*p != '\0' && p < longName + 100) p++; //Go to the end of the string
//	while (*p != '.'  && p > longName) p--; //Go back to the period
//
//	dot = p;
//
//	for (i = 0; i < 3; i++) {
//		*(name.ext + i) = *(dot + i + 1);
//		if (*(dot + i) == '\0') {
//			break;
//		}
//	}
//	*(name.ext + i) = '\0';
//
//	nameLength = dot - longName;
//	if (nameLength > 8) {
//		nameLength = 6;
//		tooLong = true;
//	}
//	for (i = 0; i < nameLength; i++) {
//		if (*(longName + i) == '.') {
//			*(name.name + i) = '\0';
//			break;
//		}
//		*(name.name + i) = *(longName + i);
//	}
//
//	if (tooLong) {
//		*(name.name + 6) = SEP_CHAR;
//		*(name.name + 7) = '1';
//	}
//
//	return nameToString(name, out);
//
//}
//
//byte nameToString(name_t name, char * string) {
//	char * p = string;
//	char * i = name.name;
//	while (*i != '\0') {
//		*p = *i;
//		p++;
//		i++;
//	}
//
//	*p = '.';
//	p++;
//
//	i = name.ext;
//	while (*i != '\0') {
//		*p = *i;
//		p++;
//		i++;
//	}
//	*p = '\0';
//	return p - string;
//}
//
//void printName(name_t name) {
//	Serial.print(name.name);
//	Serial.print(".");
//	Serial.println(name.ext);
//}


bool modifyFilename(char * filename) {
	char * p = filename;
	char * eos = p;
	bool insert = true;



	while (p < filename + 17) { //We have 13 chars, and 5 'buffer' characters to play in
		if (*p == '\0') {
			eos = p;
			if (*(p - 2) == SEP_CHAR) { //Our format is going to be "filename-n" where n is a single number. This and the next test determine if we have an extensionless file matching thr format
				if ((49 <= *(p - 1)) && (*(p - 1) < 57)) {
					(*(p - 1))++; //Move onto next value
					return true;
				}
			}
			break;
		}
		//     Serial.print(*p);
		p++;
	}

	//   Serial.println("");
	while (p >= filename) { //Step back through to list looking for "filename-n.ext"
		if (*p == '.') {
			eos = p;
			insert = true;
			if ((49 <= *(p - 1)) && (*(p - 1) < 57)) {//A number
				if (*(p - 2) == SEP_CHAR) {
					(*(p - 1))++;
					return true;
				}
			}
		}
		p--;
		//     Serial.print(*p);
	}
	//   Serial.println("");

	Serial.println(*eos);
	//Append
	if (insert == true) {
		//     Serial.println("Beep");
				//Make space
		p = eos;
		while (*p != '\0') p++;
		*(p + 2) = '\0';
		while (p >= eos) {
			//       Serial.print(*p);
			//       delay(100);
			*(p + 2) = *p;
			p--;
		}
	}
	Serial.println("");
	*eos = SEP_CHAR;
	eos++;
	*eos = '1';
	if (!insert) {
		eos++;
		*eos = '\0';
	}

	//   Serial.println( block->filename );

	return false;
}


//}