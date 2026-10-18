#include "mapsettings.h"
#include "jsonUtils.h"
#include <stdlib.h>
#include <string.h>

MapSettings mapSettings;

PostProcessData* postProcessPresets;

static __attribute__((constructor)) void init()
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
}
