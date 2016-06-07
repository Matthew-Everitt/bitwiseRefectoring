#pragma once

#include "menu.h"
class baseMenu_t : public virtual menu_t {
public:
	baseMenu_t(menu_t *  parent);
	void setNEntries(int n);
	void changeSelected(int s);
	void draw();
	void unload();

	virtual void select() = 0;
	virtual const char * getString(int i) = 0;

	int nEntries;
	int selected = 0;

private:
};