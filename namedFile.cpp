#include "namedFile.h"
#include "common.h"

extern bool SdPresent; //in systemState
extern settings_t settings;



void namedFile::detokenise(block_t * block, File * file) {
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

void namedFile::tokenise(File * file) {}




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

/*Byte handling functions, maybe should be separate file with header to include?*/
/********************************************************************************/


namedFile::namedFile() {
	for (byte i = 0; i < 5; i++) {
		currentBlock.zeroPad[i] = '\0'; /*Zero pad is to allow a longer filename (due to conflicts), and to make sure that any buffer overflows are acceptably easy to debug*/
	}
}

bool namedFile::header(namedFile * caller, byte b) {
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

bool namedFile::filename(namedFile * caller, byte b) {
	static byte pos = 0;
	if (b == 0x0D) { /*End of file name marker*/
		caller->currentBlock.filename[pos] = '\0';
		Serial.print("Detected filename is \"");
		Serial.print(caller->currentBlock.filename);
		Serial.println("\"");
		pos = 0;
		return true;
	} else {
		caller->currentBlock.filename[pos] = b;
		pos++;
		return false;
	}
}

bool namedFile::blockFlag(namedFile * caller, byte b) {
	caller->currentBlock.blockFlag.value = b;
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

bool namedFile::blockNumber(namedFile * caller, byte b) {
	//   Serial.println("blockNumber");
	return save16bitNum(b, &(caller->currentBlock.blockNumber));
}

bool namedFile::executionAddr(namedFile * caller, byte b) {
	return save16bitNum(b, &(caller->currentBlock.executionAddr));
}

bool namedFile::loadAddr(namedFile * caller, byte b) {
	return save16bitNum(b, &(caller->currentBlock.loadAddr));
}

bool namedFile::blockLength(namedFile * caller, byte b) {
	caller->currentBlock.blockLength = b;
	Serial.print("Expecting "); Serial.print((uint8_t)caller->currentBlock.blockLength); Serial.println(" + 1 bytes of data");
	return true;
}

bool namedFile::checksum(namedFile * caller, byte b) {
	caller->currentBlock.checksum = b;
	return true;
}

bool namedFile::data(namedFile * caller, byte b) {
	static byte pos = 0;
	caller->currentBlock.data[pos] = b;
	pos++;

	if (pos == (byte)(caller->currentBlock.blockLength + 1)) {
		pos = 0;
		return true;
	}

	return false;

}

bool namedFile::describeBlock(void) {
	Serial.println("Things that I know about the thing :");
	Serial.print("It is called \"");
	Serial.print(this->currentBlock.filename);
	Serial.println("\"");

	Serial.print("The block flag is ");
	Serial.print(this->currentBlock.blockFlag.value, BIN);
	Serial.println(", which tells us that :");
	Serial.print("\tThis is ");
	if (this->currentBlock.blockFlag.bits.notLastBlock == 1)  Serial.print("not ");
	Serial.println("the last block");

	Serial.print("\tThe block does ");
	if (this->currentBlock.blockFlag.bits.containsData == 0)  Serial.print("not ");
	Serial.println("contain data.");

	Serial.print("\tThis is ");
	if (this->currentBlock.blockFlag.bits.notFirstBlock == 1)  Serial.print("not ");
	Serial.println("the first block");

	Serial.print("\tSome random junk is ");
	Serial.println(this->currentBlock.blockFlag.bits.undefinedBits, BIN);

	Serial.print("The block is number ");
	Serial.println(this->currentBlock.blockNumber.value);

	Serial.print("The exec. addr. is 0x");
	Serial.println(this->currentBlock.executionAddr.value, HEX);

	Serial.print("The load. addr. is 0x");
	Serial.println(this->currentBlock.loadAddr.value, HEX);

	Serial.print("The data is ");
	Serial.print(this->currentBlock.blockLength);
	Serial.println(" bytes long");

	Serial.print("The checksum is 0x");
	Serial.println(this->currentBlock.checksum, HEX);

	Serial.print("\nWe calculate the checksum to be 0x");
	Serial.println(calculateChecksum(this->currentBlock), HEX);

	Serial.print("\tThat's a difference of 0x");
	Serial.println(calculateChecksum(this->currentBlock) ^ this->currentBlock.checksum, HEX);
	Serial.println("\n");
	return true;
	Serial.println("DUMP:");
	for (unsigned int i = 0; i < sizeof(block_t); i++) {
		Serial.print(((char *)&(this->currentBlock))[i]);
		Serial.print("\t\t");
		Serial.println(((byte *)&(this->currentBlock))[i], HEX);
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

	if (function(this,b)) pos++;
	if (nFunctions == pos) {
		pos = 0;
#ifdef reportFunctionCalls
		Serial.print("Running function finishBlock");
#endif
		finishBlock(this->currentBlock);
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
