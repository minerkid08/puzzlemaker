#include "mapsettings.h"
#include "cjson.h"
#include "dynList.h"
#include "jsonUtils.h"
#include "utils.h"
#include <stdio.h>
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

void loadMapSettingsPresets()
{
	FILE* file = fopen("postProcess.json", "rb");
	if (file == 0)
		errorf("failed to open postProcess.json\n");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);
	if (err)
		jsonParseError(data, err, "postProcess.json");

	jsonResetStack("postProcess.json");

	int arrSize = cJSON_GetArraySize(json);
	postProcessPresets = dynList_new(arrSize, sizeof(PostProcessData));

	for (int i = 0; i < arrSize; i++)
	{
		cJSON* item = jsonArrGetObject(json, i);
		PostProcessData* data = &postProcessPresets[i];

		data->name = jsonGetStr(item, "name");

		data->maxExposure = jsonGetFloat(item, "maxExposure");
		data->minExposure = jsonGetFloat(item, "minExposure");
		data->tonemapRate = jsonGetFloat(item, "tonemapRate");
		data->brightPixelPercent = jsonGetFloat(item, "brightPixelPercent");

		jsonGetVec3(item, "primaryFogColor", data->primaryFogColor);
		jsonGetVec3(item, "secondaryFogColor", data->secondaryFogColor);
		data->primaryFogColor[0] /= 255.0f;
		data->primaryFogColor[1] /= 255.0f;
		data->primaryFogColor[2] /= 255.0f;
		data->secondaryFogColor[0] /= 255.0f;
		data->secondaryFogColor[1] /= 255.0f;
		data->secondaryFogColor[2] /= 255.0f;
		data->fogStart = jsonGetFloat(item, "fogStart");
		data->fogEnd = jsonGetFloat(item, "fogEnd");
		data->fogDensity = jsonGetFloat(item, "fogDensity");
		jsonPop();
	}
	if (arrSize == 0)
		errorf("no post processing presets set\n");
	memcpy(&mapSettings.postProcess, &postProcessPresets[0], sizeof(PostProcessData));
}
