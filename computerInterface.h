#pragma once
class ComputerInterface {
public:
	//Called by the ISR
	virtual void recordChange(void) = 0;

	//Indicates that a byte has been received and is ready to be processed.
	bool newByteAvaliable = false;
	//The value of said received byte.
	byte data;


	//Indicates that there is space in the transmit buffer.
	bool bufferAvaliable = false;

	//Enter a byte into the transmit queue
	//Returns true if ok, false if unable to send for whatever reason

	virtual bool sendByte(byte b) = 0;

	//Inform the interface that the file being sent has ended. Ideally this will also be detected with a timeout, but why rely on that?
	virtual void endTransmission() = 0;
};
