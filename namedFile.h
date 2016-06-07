#ifndef __namedFile_h__

#define __namedFile_h__

#include "common.h"
#include "Arduino.h"
#include "FileTools.h"


typedef union {
	byte value;
	struct bits_t {
		byte undefinedBits : 5;
		bit  notFirstBlock : 1;
		bit  containsData : 1;
		bit  notLastBlock : 1;
	} bits;
} binary_t;

typedef struct {
	byte low;
	byte high;
} twoBytes_t;

typedef union {
	twoBytes_t bytes;
	uint16_t value;
} sixteenBit_t;

/*A datatype to store everything worth knowing about a block of data*/
typedef struct {
	char filename[13]; /*The supplied file name*/
	char zeroPad[5];   /*A bit of padding so we have a chance of understanding what the printed string is if we miss a null termination*/


	/*In theory this allows us to treat the blockFlag as  a byte ( [struct].blockFlag.value ), or read (or write - careful!) the different fields ( [struct].blockFlag.bits.[field]). Lets see how long before that breaks!*/
	binary_t blockFlag;

	sixteenBit_t blockNumber;

	byte blockLength;

	sixteenBit_t executionAddr;
	sixteenBit_t loadAddr;

	byte data[256];

	byte checksum;

} block_t;
void detokenise(block_t * block, File * file);
void tokenise(File * file);
class namedFile : public format {
	/*Byte handling functions, maybe should be seperate file with header to include?*/
	/********************************************************************************/

	static bool header(byte b);

	static bool filename(byte b);
	static bool blockFlag(byte b);

	static bool save16bitNum(byte b, sixteenBit_t * target);

	static bool blockNumber(byte b);
	static bool executionAddr(byte b);
	static bool loadAddr(byte b);
	static bool blockLength(byte b);
	static bool checksum(byte b);
	static bool data(byte b);


	bool describeBlock(void);
	/********************************************************************************/

	void RX(byte b, String defaultFilename);
	void TX(File * file);
	void finishBlock(block_t block);


	byte calculateChecksum(block_t block);
};
#endif