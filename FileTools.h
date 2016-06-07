#ifndef __FileTools_h__
#define __FileTools_h__

#include "common.h"
#include "Arduino.h"

void setupSD();
int bytesToString(float bytes, char * buffer, int n);
//byte longNameToShortName(char * longName, char * out);
//byte nameToString(name_t name, char * string);
//void printName(name_t name);

bool modifyFilename(char * filename);
float freeSpace();
#endif