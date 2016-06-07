#pragma once
#include "baseMenu.h"

class settingsMenu_t : public virtual baseMenu_t {
public:
	settingsMenu_t(menu_t * parent);
	const char * getString(int index);
	void select();
	void load();
private:
	enum {
		OVERWRITE,
		DETOKENISE,
		TOKENISE,
		SEND_NAMED,
		N_SETTINGS,
	};
	const char * boolToString(bool b);

	static const int bufferLen = 64; //Display is only 64 pixels wide, so this is pesimistic, but we have RAM, and this isn't the kind of problem that matters unless we need to move to new hardware, in which case we need to think long and hard about these kind of things
	char buffer[bufferLen];
};