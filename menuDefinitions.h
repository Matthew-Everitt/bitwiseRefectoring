#include "baseMenu.h"
#include "fileMenu.h"
#include "normalMenu.h"
#include "settingsMenu.h"
#include "menu.h"
//Actual menus
static normalMenu_t topMenu(NULL);
static fileMenu_t filesMenu(&topMenu);
static settingsMenu_t settingsMenu(&topMenu);

//Menu resources
const char * topStrings[2] = { "Files", "Settings" };
menu_t * topMenus[2] = { &filesMenu, &settingsMenu };

void setupMenus() {
	topMenu.setStrings(topStrings);
	topMenu.setMenus(topMenus);
	topMenu.setNEntries(2);
}
