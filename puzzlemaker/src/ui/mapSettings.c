#include "mapsettings.h"
#include "dynList.h"
#include "packages/theme.h"
#include <string.h>

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

	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;

	igBegin("Map Settings", &open, ImGuiWindowFlags_NoDocking);

	if (igBeginCombo("Map Theme", currentTheme->name, 0))
	{
		int len = dynList_size(themeList);
		for (int i = 0; i < len; i++)
		{
			Theme* theme = &themeList[i];
			char selected = (currentTheme == theme);
			if (igSelectable_Bool(theme->name, selected, 0, zero))
			{
				mapSettings.postProcessPreset = 0;
        setTheme(theme);
			}
			if (selected)
				igSetItemDefaultFocus();
		}
		igEndCombo();
	}
	igSeparatorText("Game Rules");
	igCheckbox("enable fall damage", (bool*)&mapSettings.boots);
	igCheckbox("enable health regen", (bool*)&mapSettings.regen);
	igInputInt("max health", &mapSettings.maxHealth, 1, 1, 1);

	igCombo_Str("staring portalgun", &mapSettings.portalGun, "None\0Orange\0Blue\0Blue+Orange\0Potato\0", -1);
	igCombo_Str("staring paintgun", &mapSettings.paintGun, "None\0Basic\0Full\0", -1);
	igCombo_Str("game type", &mapSettings.gameType,
				"Default\0Coop\0Coop with guns\0Coop versus\0Coop versus with guns\0", -1);

	igSeparatorText("Post Processing");

	PostProcessData* data = &mapSettings.postProcess;

  PostProcessData* presets = currentTheme->postProcessPresets;

	if (igBeginCombo("preset", presets[mapSettings.postProcessPreset].name, 0))
	{
		int len = dynList_size(presets);
		for (int i = 0; i < len; i++)
		{
			PostProcessData* preset = &presets[i];
			char selected = (mapSettings.postProcessPreset == i);
			if (igSelectable_Bool(preset->name, selected, 0, zero))
			{
				mapSettings.postProcessPreset = i;
				memcpy(data, preset, sizeof(PostProcessData));
			}
			if (selected)
				igSetItemDefaultFocus();
		}
		igEndCombo();
	}

	igDragFloat("max exposure", &data->maxExposure, 1, 0, 9999, "%.2f", 0);
	igDragFloat("min exposure", &data->minExposure, 1, 0, 9999, "%.2f", 0);
	igDragFloat("tonemap rate", &data->tonemapRate, 1, 0, 9999, "%.2f", 0);
	igDragFloat("bright pixel %", &data->brightPixelPercent, 1, 0, 9999, "%.2f", 0);
	igColorEdit3("primary fog color", data->primaryFogColor, 0);
	igColorEdit3("secondary fog color", data->primaryFogColor, 0);
	igDragFloat("fog start", &data->fogStart, 1, 0, 9999, "%.2f", 0);
	igDragFloat("fog end", &data->fogEnd, 1, 0, 9999, "%.2f", 0);
	igDragFloat("fog density", &data->fogDensity, 1, 0, 1, "%.5f", 0);

	igEnd();
}
