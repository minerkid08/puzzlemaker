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
		igCheckbox("Enable Fall Damage", (bool*)&mapSettings.boots);
		igCheckbox("Enable Health Regen", (bool*)&mapSettings.regen);
		igInputInt("Max Health", &mapSettings.maxHealth, 1, 1, 1);

		igCombo_Str("Staring Portalgun", &mapSettings.portalGun, "None\0Orange\0Blue\0Blue+Orange\0Potato\0", -1);
		igCombo_Str("Staring Paintgun", &mapSettings.paintGun, "None\0Basic\0Full\0", -1);
		igCombo_Str("Game Type", &mapSettings.gameType,
					"Default\0Coop\0Coop with Guns\0Coop Versus\0Coop Versus with Guns\0", -1);
		igTreePop();
	}

	if (igTreeNodeEx_Str("Post Processing", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
	{
		PostProcessData* data = &mapSettings.postProcess;

		PostProcessData* presets = currentTheme->postProcessPresets;

		if (igBeginCombo("Preset", presets[mapSettings.postProcessPreset].name, 0))
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
		igPushID_Str("enableTonemap");
		igCheckbox("Enabled", (bool*)&data->enableTonemap);
		igPopID();
		igBeginDisabled(data->enableTonemap == 0);
		igDragFloat("Max Exposure", &data->maxExposure, 1, 0, 9999, "%.2f", 0);
		igDragFloat("Min Exposure", &data->minExposure, 1, 0, 9999, "%.2f", 0);
		igDragFloat("Tonemap Rate", &data->tonemapRate, 1, 0, 9999, "%.2f", 0);
		igDragFloat("Bright Pixel %", &data->brightPixelPercent, 1, 0, 9999, "%.2f", 0);
		igEndDisabled();

		igSeparatorText("Fog");
		igPushID_Str("enableFog");
		igCheckbox("Enabled", (bool*)&data->enableFog);
		igPopID();
		igBeginDisabled(data->enableFog == 0);
		igColorEdit3("Primary Color", data->primaryFogColor, 0);
		igColorEdit3("Secondary Color", data->primaryFogColor, 0);
		igDragFloat("Start", &data->fogStart, 1, 0, 9999, "%.2f", 0);
		igDragFloat("End", &data->fogEnd, 1, 0, 9999, "%.2f", 0);
		igDragFloat("Density", &data->fogDensity, 0.001, 0, 1, "%.5f", 0);
		igEndDisabled();

		igSeparatorText("Color Correction");
		igPushID_Str("enableColor");
		igCheckbox("Enabled", (bool*)&data->enableColorCorrection);
		igPopID();
		igBeginDisabled(data->enableColorCorrection == 0);
		igDragFloat("Min Falloff", &data->minFalloff, 1, -1, 9999, "%.2f", 0);
		igDragFloat("Max Falloff", &data->maxFalloff, 1, -1, 9999, "%.2f", 0);
		igDragFloat("Max Weight", &data->maxWeight, 0.01, 0, 1, "%.2f", 0);
		igDragFloat("Fade In Duration", &data->fadeInDuration, 1, 0, 9999, "%.2f", 0);
		igDragFloat("Fade Out Duration", &data->fadeOutDuration, 1, 0, 9999, "%.2f", 0);
		igInputText("Filename", data->filename, 256, 0, 0, 0);
		igEndDisabled();

		igTreePop();
	}

	igEnd();
}
