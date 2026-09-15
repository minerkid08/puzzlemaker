#include "mapsettings.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"

static bool open = 0;

void openMapSettingsUi()
{
	open = 1;
}

void renderMapSettingsUi()
{
	if (open == 0)
		return;

	igBegin("Map Settings", &open, ImGuiWindowFlags_NoDocking);
  
  igCheckbox("enable fall damage", (bool*)&mapSettings.boots);
  igCheckbox("enable health regen", (bool*)&mapSettings.regen);
  igInputInt("max health", &mapSettings.maxHealth, 1, 1, 1);

  igCombo_Str("staring portalgun", &mapSettings.portalGun, "None\0Blue\0Orange\0Blue+Orange\0Potato\0", -1);
  igCombo_Str("staring paintgun", &mapSettings.paintGun, "None\0Basic\0Full\0", -1);
  igCombo_Str("game type", &mapSettings.gameType, "Default\0Coop\0Coop with guns\0Coop versus\0Coop versus with guns\0", -1);
	igEnd();
}
