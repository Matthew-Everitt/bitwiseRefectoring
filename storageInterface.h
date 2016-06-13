#pragma once

class storageInterface {
public:
	virtual void RX(byte b, String defaultFilename = "blah") = 0; //We receive data from the Atom one byte at a time
	virtual void TX(File * file) = 0; //But we might as well send an entire file at once.
};
