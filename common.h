#pragma once

#include <U8glib.h>
#include <Encoder.h>
#include <Arduino.h>
#include <SPI.h>
#include <stdbool.h>
#include <SdFat.h>

#include "menu.h"




//These are more dangerous than the Arduino default using typeof, but visual studio objects to them at some level.
//Main thing to remember is that the arguments to these get evaluated repeatedly, so something like x++ is probably not a smart argument.
#undef max
#define max(a,b) ( (a)>(b)?(a):(b) )
#undef min
#define min(a,b) ( (a)<(b)?(a):(b) )

#define isOdd(x) ((x%2)==1)
#define isEven(x) ((x%2)==0)

#include "pinout.h"
#include "debugLevels.h"


//#define carrierTimer Timer1


extern SdFat SD;
typedef uint8_t byte;
typedef uint8_t bit;






//typedef struct {
//	char name[9];
//	char ext[4];
//} name_t;





typedef struct {
	bool overwrite = false;
	bool detokenise = true;
	bool tokenise = true;
	bool sendNamed = true;
} settings_t;


