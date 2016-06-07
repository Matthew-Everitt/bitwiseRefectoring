#include "namedFile.h"
#include "common.h"

extern bool SdPresent; //in systemState
extern settings_t settings;

block_t currentBlock;

void detokenise(block_t * block, File * file) {
	byte * p = block->data;
	byte * end = block->data + block->blockLength;

	static bool needFirstN = false;
	static bool needSecondN = false;

	char buf[6];
	int count;
	sixteenBit_t lineNo;

	while (p < end) {
		if (needFirstN) {
			if (*p == 0xFF) break; //0xFF as high byte signifies the end of a file

			lineNo.bytes.high = *p;
			needFirstN = false;
			needSecondN = true;
			p++;
			continue;
		}
		if (needSecondN) {
			lineNo.bytes.low = *p;
			needSecondN = false;
			count = sprintf(buf, "%u", lineNo.value);
			// 	Serial.print("Line # ");
			// 	Serial.println( buf );
			file->write(buf, count);
			p++;
			continue;
		}

		if (*p == 0x0D) { //0x0D is the carrage return used to signify the start of a line
			needFirstN = true;
		}


		file->write(*p);
		p++;
	}
}

void tokenise(File * file) {}




// class namedFile : public format{
//   
byte namedFile::calculateChecksum(block_t block) {
	uint32_t sum = 4 * 0x2A; /*Four header bytes*/
#ifdef detailedChecksum
	Serial.println("");
	Serial.print("header :  ");
	Serial.println(sum);
#endif
	uint16_t i = 0;
	while (block.filename[i] != '\0') {
		sum += block.filename[i];
		i++;
	}

	sum += 0x0D; //End of filename marker
#ifdef detailedChecksum
	Serial.print("filename + \\n :  ");
	Serial.println(sum);
#endif
	sum += block.blockFlag.value;
#ifdef detailedChecksum
	Serial.print("flag :  ");
	Serial.println(sum);
#endif  
	sum += block.blockNumber.bytes.high;
	sum += block.blockNumber.bytes.low;
#ifdef detailedChecksum
	Serial.print("number :  ");
	Serial.println(sum);
#endif  
	sum += block.blockLength;
#ifdef detailedChecksum
	Serial.print("len :  ");
	Serial.println(sum);
#endif  

	sum += block.executionAddr.bytes.high;
	sum += block.executionAddr.bytes.low;

	sum += block.loadAddr.bytes.high;
	sum += block.loadAddr.bytes.low;
#ifdef detailedChecksum
	Serial.print("addr :  ");
	Serial.println(sum);
#endif
	for (i = 0; i <= block.blockLength; i++) {
		sum += block.data[i];
	}
#ifdef detailedChecksum
	Serial.print("data :  ");
	Serial.println(sum);
#endif
	return (byte)(sum % 256);
}

/*Byte handling functions, maybe should be seperate file with header to include?*/
/********************************************************************************/

bool namedFile::header(byte b) {
	static uint8_t count = 0;

	/*The header is defined as 4 * 0x2A, so if we get anything else print a warning but ignore it*/
	if (b != 0x2a) {
		Serial.print("Non header byte (\""); Serial.print(b); Serial.print("\" - 0x"); Serial.print(b, HEX); Serial.println(") when header expected");
		return false;
	}

	count++;
	if (count == 4) {
		count = 0;
		return true;
	} else {
		return false;
	}
}

bool namedFile::filename(byte b) {
	static byte pos = 0;
	if (b == 0x0D) { /*End of file name marker*/
		currentBlock.filename[pos] = '\0';
		Serial.print("Detected filename is \"");
		Serial.print(currentBlock.filename);
		Serial.println("\"");
		pos = 0;
		return true;
	} else {
		currentBlock.filename[pos] = b;
		pos++;
		return false;
	}
}

bool namedFile::blockFlag(byte b) {
	currentBlock.blockFlag.value = b;
	return true;
}

bool namedFile::save16bitNum(byte b, sixteenBit_t * target) {
	static bool firstByte = true;

#ifdef twoByteNumbers

	Serial.print("Reading two byte #, firstByte is "); Serial.println(firstByte);
	Serial.print("Before doing anything the target (0x");
	Serial.print((uint)&(target->value), HEX);
	Serial.println(") is ");
	Serial.print(target->value);
	Serial.print(" ( ");
	Serial.print(target->bytes.high, BIN);
	Serial.print(" ");
	Serial.print(target->bytes.low, BIN);
	Serial.println(")");
#endif
	if (firstByte) {
		target->bytes.high = b;
		firstByte = false;
	} else {
		target->bytes.low = b;
		firstByte = true;
	}
#ifdef chatty
	Serial.print("After adding the byte (");
	Serial.print(b, BIN);
	Serial.println(") the target is ");
	Serial.print(target->value);
	Serial.print(" ( ");
	Serial.print(target->bytes.high, BIN);
	Serial.print(" ");
	Serial.print(target->bytes.low, BIN);
	Serial.println(")");
#endif  
	return firstByte;
}

bool namedFile::blockNumber(byte b) {
	//   Serial.println("blockNumber");
	return save16bitNum(b, &(currentBlock.blockNumber));
}

bool namedFile::executionAddr(byte b) {
	return save16bitNum(b, &(currentBlock.executionAddr));
}

bool namedFile::loadAddr(byte b) {
	return save16bitNum(b, &(currentBlock.loadAddr));
}

bool namedFile::blockLength(byte b) {
	currentBlock.blockLength = b;
	Serial.print("Expecting "); Serial.print((uint8_t)currentBlock.blockLength); Serial.println(" + 1 bytes of data");
	return true;
}

bool namedFile::checksum(byte b) {
	currentBlock.checksum = b;
	return true;
}

bool namedFile::data(byte b) {
	static byte pos = 0;
	currentBlock.data[pos] = b;
	pos++;

	if (pos == (byte)(currentBlock.blockLength + 1)) {
		pos = 0;
		return true;
	}

	return false;

}

bool namedFile::describeBlock(void) {
	Serial.println("Things that I know about the thing :");
	Serial.print("It is called \"");
	Serial.print(currentBlock.filename);
	Serial.println("\"");

	Serial.print("The block flag is ");
	Serial.print(currentBlock.blockFlag.value, BIN);
	Serial.println(", which tells us that :");
	Serial.print("\tThis is ");
	if (currentBlock.blockFlag.bits.notLastBlock == 1)  Serial.print("not ");
	Serial.println("the last block");

	Serial.print("\tThe block does ");
	if (currentBlock.blockFlag.bits.containsData == 0)  Serial.print("not ");
	Serial.println("contain data.");

	Serial.print("\tThis is ");
	if (currentBlock.blockFlag.bits.notFirstBlock == 1)  Serial.print("not ");
	Serial.println("the first block");

	Serial.print("\tSome random junk is ");
	Serial.println(currentBlock.blockFlag.bits.undefinedBits, BIN);

	Serial.print("The block is number ");
	Serial.println(currentBlock.blockNumber.value);

	Serial.print("The exec. addr. is 0x");
	Serial.println(currentBlock.executionAddr.value, HEX);

	Serial.print("The load. addr. is 0x");
	Serial.println(currentBlock.loadAddr.value, HEX);

	Serial.print("The data is ");
	Serial.print(currentBlock.blockLength);
	Serial.println(" bytes long");

	Serial.print("The checksum is 0x");
	Serial.println(currentBlock.checksum, HEX);

	Serial.print("\nWe calculate the checksum to be 0x");
	Serial.println(calculateChecksum(currentBlock), HEX);

	Serial.print("\tThat's a difference of 0x");
	Serial.println(calculateChecksum(currentBlock) ^ currentBlock.checksum, HEX);
	Serial.println("\n");
	return true;
	Serial.println("DUMP:");
	for (unsigned int i = 0; i < sizeof(block_t); i++) {
		Serial.print(((char *)&currentBlock)[i]);
		Serial.print("\t\t");
		Serial.println(((byte *)&currentBlock)[i], HEX);
	}

	return false;
}

/********************************************************************************/

void namedFile::TX(File * file) {

}

void namedFile::RX(byte b, String defaultFilename = "default.file") {
	const uint8_t nFunctions = 9;
	byteHandler byteFunctions[nFunctions] = { &(namedFile::header), &namedFile::filename, &namedFile::blockFlag, &namedFile::blockNumber, &namedFile::blockLength, &namedFile::executionAddr, &namedFile::loadAddr, &namedFile::data, &namedFile::checksum };
	static uint8_t pos = 0;

#ifdef reportFunctionCalls
	const char * byteFunctionNames[nFunctions] = { "header", "filename", "blockFlag", "blockNumber", "blockLength", "executionAddr", "loadAddr", "data", "checksum" };
	Serial.print("Running function "); Serial.println(byteFunctionNames[pos]);
#endif

	byteHandler function = byteFunctions[pos];

#ifdef reportFunctionCalls
	Serial.print("Out of function "); Serial.println(byteFunctionNames[pos]);
#endif

	if (function(b)) pos++;
	if (nFunctions == pos) {
		pos = 0;
#ifdef reportFunctionCalls
		Serial.print("Running function finishBlock");
#endif
		finishBlock(currentBlock);
#ifdef reportFunctionCalls
		Serial.print("Out of function finishBlock");
#endif

	}
}



void namedFile::finishBlock(block_t block) {
#ifdef reportBlockDetails
	describeBlock();
#endif
	static File file;
	if (SdPresent) {
		if (block.blockFlag.bits.notFirstBlock == 0) { //If this is the first block
//         longNameToShortName( block.filename, block.filename );

			if (settings.overwrite == false) {
				while (true) {

					if (SD.exists(block.filename)) {
#ifdef verboseFile
						Serial.print("File \"");
						Serial.print(block.filename);
						Serial.println("\" already exists");
#endif
						//Should really rewrite modifyFilename to ensure it gives an 8.3 filename rather than manging it afterwards.
						modifyFilename(block.filename);
						// 	      longNameToShortName( block.filename, block.filename );

					} else {
#ifdef verboseFile
						Serial.print("File \"");
						Serial.print(block.filename);
						Serial.println("\" doesn't seem to exist");
#endif
						break;

					}
				}
			}
			Serial.print("Writing to file \"");
			Serial.print(block.filename);
			Serial.println("\"");
			file = SD.open(block.filename, FILE_WRITE);
		}
		if (settings.detokenise) {
			detokenise(&block, &file);
		} else {
			file.write(block.data, block.blockLength);
		}
		if (block.blockFlag.bits.notLastBlock == 0) { //This is the last block
#ifdef verboseFile
			Serial.println("Closing file");
			Serial.println();
			Serial.println();
			Serial.println();
#endif
			file.close();
		}
	}
}
