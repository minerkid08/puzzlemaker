#include "packages/theme.h"
#include "antline/antline.h"
#include "assetManager.h"
#include "cjson.h"
#include "dynList.h"
#include "jsonUtils.h"
#include "packages/package.h"
#include "utils.h"
#include "voxel/voxelConfig.h"
#include <stdio.h>
#include <string.h>

Theme* currentTheme;
Theme* themeList;

static __attribute__((constructor)) void init()
{
	themeList = dynList_new(0, sizeof(Theme));
}

static void loadVoxelConfig(Theme* theme, cJSON* json)
{
	theme->voxelCfg.nodraw = jsonGetStrD(json, "nodraw");
	theme->voxelCfg.backstage = jsonGetStrD(json, "backstage");

	theme->voxelCfg.blackFloor = jsonGetStrD(json, "blackFloor");
	theme->voxelCfg.blackWall = jsonGetStrD(json, "blackWall");
	theme->voxelCfg.blackCeiling = jsonGetStrD(json, "blackCeiling");
	theme->voxelCfg.whiteFloor = jsonGetStrD(json, "whiteFloor");
	theme->voxelCfg.whiteWall = jsonGetStrD(json, "whiteWall");
	theme->voxelCfg.whiteCeiling = jsonGetStrD(json, "whiteCeiling");

	theme->voxelCfg.blackFloorMini = jsonGetStrD(json, "blackFloorMini");
	theme->voxelCfg.blackWallMini = jsonGetStrD(json, "blackWallMini");
	theme->voxelCfg.blackCeilingMini = jsonGetStrD(json, "blackCeilingMini");
	theme->voxelCfg.whiteFloorMini = jsonGetStrD(json, "whiteFloorMini");
	theme->voxelCfg.whiteWallMini = jsonGetStrD(json, "whiteWallMini");
	theme->voxelCfg.whiteCeilingMini = jsonGetStrD(json, "whiteCeilingMini");

	const char* blackEditor = jsonGetStr(json, "blackEditor");
	theme->voxelCfg.blackEditor = assetManagerLoadTexture(blackEditor);

	const char* whiteEditor = jsonGetStr(json, "whiteEditor");
	theme->voxelCfg.whiteEditor = assetManagerLoadTexture(whiteEditor);

	const char* blackMiniEditor = jsonGetStr(json, "blackMiniEditor");
	theme->voxelCfg.blackMiniEditor = assetManagerLoadTexture(blackMiniEditor);

	const char* whiteMiniEditor = jsonGetStr(json, "whiteMiniEditor");
	theme->voxelCfg.whiteMiniEditor = assetManagerLoadTexture(whiteMiniEditor);
}

void loadAntlineConfig(Theme* theme, cJSON* json)
{
	theme->antlineCfg.dotSize = jsonGetInt(json, "dotSize");
	theme->antlineCfg.checkSize = jsonGetInt(json, "checkSize");

	theme->antlineCfg.antlineTex = jsonGetStrD(json, "lineTex");
	theme->antlineCfg.antlineCornerTex = jsonGetStrD(json, "cornerTex");
	theme->antlineCfg.antlineCheckTex = jsonGetStrD(json, "checkTex");

	theme->antlineCfg.antlineWidth = jsonGetInt(json, "lineDotCountx");
	theme->antlineCfg.antlineHeight = jsonGetInt(json, "lineDotCounty");
	theme->antlineCfg.antlineCornerWidth = jsonGetInt(json, "cornerDotCountx");
	theme->antlineCfg.antlineCornerHeight = jsonGetInt(json, "cornerDotCounty");

	const char* antlineTex = jsonGetStr(json, "editorAntlineTex");
	theme->antlineCfg.editorAntlineTex = assetManagerLoadTexture(antlineTex);
	const char* antlineActiveTex = jsonGetStr(json, "editorAntlineActiveTex");
	theme->antlineCfg.editorAntlineActiveTex = assetManagerLoadTexture(antlineActiveTex);

	const char* checkTex = jsonGetStr(json, "editorCheckTex");
	theme->antlineCfg.editorCheckTex = assetManagerLoadTexture(checkTex);
	const char* checkActiveTex = jsonGetStr(json, "editorCheckActiveTex");
	theme->antlineCfg.editorCheckActiveTex = assetManagerLoadTexture(checkActiveTex);

	theme->antlineCfg.editorCheckSize = (float)theme->antlineCfg.checkSize / 128.0f;
	theme->antlineCfg.editorDotSize = (float)theme->antlineCfg.dotSize / 128.0f;
}

void loadPostProcessingPresets(Theme* theme, cJSON* json)
{
	int arrSize = cJSON_GetArraySize(json);
	theme->postProcessPresets = dynList_new(arrSize, sizeof(PostProcessData));

	for (int i = 0; i < arrSize; i++)
	{
		cJSON* item = jsonArrGetObject(json, i);
		PostProcessData* data = &theme->postProcessPresets[i];

		data->name = jsonGetStrD(item, "name");

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
}

void loadTheme(const char* filename)
{
	printf("[package loader] loading theme %s\n", filename);
	char path[64];
	snprintf(path, 64, "themes/%s", filename);

  int dataSize;
  char* data = readPackageFile(currentPackage, path, &dataSize);
	data[dataSize] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);
	if (err)
		jsonParseError(data, err, path);

	jsonResetStack(path);

	int themeCount = dynList_size(themeList);
	dynList_resize((void**)&themeList, themeCount + 1);
	Theme* theme = &themeList[themeCount];
	cJSON* voxel = jsonGetObject(json, "voxel");
	loadVoxelConfig(theme, voxel);
  jsonPop();
	cJSON* antline = jsonGetObject(json, "antline");
	loadAntlineConfig(theme, antline);
  jsonPop();
	cJSON* postProcessing = jsonGetArray(json, "postProcessing");
	loadPostProcessingPresets(theme, postProcessing);
  jsonPop();

  int l = strlen(filename);

  char* name = malloc(l - 4);
  snprintf(name, l - 4, "%s", filename);

  theme->name = name;
}

void setTheme(Theme* theme)
{
	currentTheme = theme;
	voxelConfig = &theme->voxelCfg;
	antlineConfig = &theme->antlineCfg;
}
