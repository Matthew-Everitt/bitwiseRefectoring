#pragma once
class computerInterface {
public:
	virtual void recordChange(void) = 0;
	bool newByte;
	byte data;
};
