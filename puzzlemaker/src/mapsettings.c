#include "mapsettings.h"
#include "jsonUtils.h"
#include <stdlib.h>
#include <string.h>

MapSettings mapSettings;

PostProcessData* postProcessPresets;

#ifdef __linux__
__attribute__((constructor)) static void mapSettingsInit()
#else
void mapSettingsInit()
#endif
{
	mapSettings.name = malloc(256);
	mapSettings.name[0] = 0;
	mapSettings.boots = 0;
	mapSettings.regen = 1;
	mapSettings.portalGun = 3;
	mapSettings.paintGun = 0;
	mapSettings.maxHealth = 100;
	mapSettings.gameType = 0;

	mapSettings.postProcessPreset = 0;

	PostProcessData* data = &mapSettings.postProcess;
	memset(data, 0, sizeof(PostProcessData));
	data->filename = malloc(256);
	data->filename[0] = 0;
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*mapSettingsInitPtr)(void) = mapSettingsInit;

#pragma comment(linker, "/include:mapSettingsInitPtr")
#endif
