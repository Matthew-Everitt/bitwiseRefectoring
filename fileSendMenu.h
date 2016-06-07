#pragma once
#include "baseMenu.h"
#include "menu.h"
class fileSendMenu_t : public virtual baseMenu_t {
	friend class fileMenu_t; //this class will probably be created and 'owned' by a fileMenu_t, which will need to be able to set itself as a parent
public:
	fileSendMenu_t(menu_t * parent);
	const char * getString(int index);
	void select();
	void load();
private:
	enum {
		SEND_DEFAULT,
		SEND_NAMED_TOKENISED,
		SEND_UNAMED_TOKENISED,
		SEND_NAMED_NOT_TOKENISED,
		SEND_UNAMED_NOT_TOKENISED,
		N_SEND_OPTIONS,
	};



	static const int bufferLen = 64; //Display is only 64 pixels wide, so this is pessimistic, but we have RAM, and this isn't the kind of problem that matters unless we need to move to new hardware, in which case we need to think long and hard about these kind of things
	char buffer[bufferLen];
};
