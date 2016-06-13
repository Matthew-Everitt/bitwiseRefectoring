#pragma once

#include "common.h"
#include "Arduino.h"
#include "FileTools.h"

#include "storageInterface.h"



class namedFile : public storageInterface {
	//Probably generic enough to go into some other class / namespace
	typedef struct {
		byte low;
		byte high;
	} twoBytes_t;

	typedef union {
		twoBytes_t bytes;
		uint16_t value;
	} sixteenBit_t;

	//Probably generic enough to do something with, possibly a more general class that this can inherit
	/*Prototype for the byte handing functions. Functions take the byte as input, and return 'finished', so true means move on to next function, false means pass next byte to this function*/
	typedef bool(*byteHandler)(namedFile * caller, byte b);


	//Stuff that only really applies to namedFiles
	typedef union {
		byte value;
		struct bits_t {
			byte undefinedBits : 5;
			bit  notFirstBlock : 1;
			bit  containsData : 1;
			bit  notLastBlock : 1;
		} bits;
	} namedFileBlockFlag_t;

	/*A data type to store everything worth knowing about a block of data*/
	typedef struct {
		char filename[13]; /*The supplied file name*/
		char zeroPad[5];   /*A bit of padding so we have a chance of understanding what the printed string is if we miss a null termination*/
		namedFileBlockFlag_t blockFlag; /*In theory this allows us to treat the blockFlag as  a byte ( [struct].blockFlag.value ), or read (or write - careful!) the different fields ( [struct].blockFlag.bits.[field]). Lets see how long before that breaks!*/
		sixteenBit_t blockNumber;
		byte blockLength;
		sixteenBit_t executionAddr;
		sixteenBit_t loadAddr;
		byte data[256];
		byte checksum;
	} block_t;

	block_t currentBlock;

	//Detokenise could be made a wrapper for an array based implementation, and then these two could be made more generic rather than class specific.
	void detokenise(block_t * block, File * file);
	void tokenise(File * file);

	/*Byte handling functions*/
	/********************************************************************************/
	static bool header(namedFile * caller, byte b);
	static bool filename(namedFile * caller, byte b);
	static bool blockFlag(namedFile * caller, byte b);
	static bool save16bitNum(byte b, sixteenBit_t * target);
	static bool blockNumber(namedFile * caller, byte b);
	static bool executionAddr(namedFile * caller, byte b);
	static bool loadAddr(namedFile * caller, byte b);
	static bool blockLength(namedFile * caller, byte b);
	static bool checksum(namedFile * caller, byte b);
	static bool data(namedFile * caller, byte b);
	/********************************************************************************/

	//Named file specific
	bool describeBlock(void);
	void finishBlock(block_t block);
  byte calculateChecksum(block_t block);


public:
	namedFile();
	void RX(byte b, String defaultFilename);
	void TX(File * file);
};
