#pragma once

#include "baseMenu.h"
#include "common.h"

class normalMenu_t : public virtual baseMenu_t {
public:
	normalMenu_t(menu_t * parent);
	const char * getString(int index);
	void select();
	void load();
	void setMenus(menu_t ** menus);
	void setStrings(const char ** strings);

private:
	const char ** strings;
	menu_t ** menus;
};
