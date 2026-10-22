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
	if (igTreeNodeEx_Str("Game Rules", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
	{
		igCheckbox("enable fall damage", (bool*)&mapSettings.boots);
		igCheckbox("enable health regen", (bool*)&mapSettings.regen);
		igInputInt("max health", &mapSettings.maxHealth, 1, 1, 1);

		igCombo_Str("staring portalgun", &mapSettings.portalGun, "None\0Orange\0Blue\0Blue+Orange\0Potato\0", -1);
		igCombo_Str("staring paintgun", &mapSettings.paintGun, "None\0Basic\0Full\0", -1);
		igCombo_Str("game type", &mapSettings.gameType,
					"Default\0Coop\0Coop with guns\0Coop versus\0Coop versus with guns\0", -1);
		igTreePop();
	}

	if (igTreeNodeEx_Str("Post Processing", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
	{
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
					char* filename = data->filename;
					memcpy(data, preset, sizeof(PostProcessData));
					data->filename = filename;
					strncpy(filename, preset->filename, 256);
				}
				if (selected)
					igSetItemDefaultFocus();
			}
			igEndCombo();
		}

		igSeparatorText("Tonemap");
		igDragFloat("max exposure", &data->maxExposure, 1, 0, 9999, "%.2f", 0);
		igDragFloat("min exposure", &data->minExposure, 1, 0, 9999, "%.2f", 0);
		igDragFloat("tonemap rate", &data->tonemapRate, 1, 0, 9999, "%.2f", 0);
		igDragFloat("bright pixel %", &data->brightPixelPercent, 1, 0, 9999, "%.2f", 0);

		igSeparatorText("Fog");
		igColorEdit3("primary color", data->primaryFogColor, 0);
		igColorEdit3("secondary color", data->primaryFogColor, 0);
		igDragFloat("start", &data->fogStart, 1, 0, 9999, "%.2f", 0);
		igDragFloat("end", &data->fogEnd, 1, 0, 9999, "%.2f", 0);
		igDragFloat("density", &data->fogDensity, 1, 0, 1, "%.5f", 0);

		igSeparatorText("Color Correction");
		igDragFloat("min falloff", &data->minFalloff, 1, -1, 9999, "%.2f", 0);
		igDragFloat("max falloff", &data->maxFalloff, 1, -1, 9999, "%.2f", 0);
		igDragFloat("max weight", &data->maxWeight, 0.01, 0, 1, "%.2f", 0);
		igDragFloat("fade in duration", &data->fadeInDuration, 1, 0, 9999, "%.2f", 0);
		igDragFloat("fade out duration", &data->fadeOutDuration, 1, 0, 9999, "%.2f", 0);
		igInputText("filename", data->filename, 256, 0, 0, 0);

		igTreePop();
	}

	igEnd();
}
