#include "packages/theme.h"
#include "antline/antline.h"
#include "assetManager.h"
#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "jsonUtils.h"
#include "mapsettings.h"
#include "packages/package.h"
#include "utils.h"
#include "voxel/voxelConfig.h"
#include <stdio.h>
#include <string.h>

Theme* currentTheme;
Theme* themeList;

#ifdef __linux__
__attribute__((constructor)) static void themeInit()
#else
void themeInit()
#endif
{
	themeList = dynList_new(0, sizeof(Theme));
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*themeInitPtr)(void) = themeInit;

#pragma comment(linker, "/include:themeInitPtr")
#endif

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

	char flags = 0;
	const char* blackEditor = jsonGetStr(json, "blackEditor");
	theme->voxelCfg.blackEditor = assetManagerLoadTexture(blackEditor, 0);

	const char* whiteEditor = jsonGetStr(json, "whiteEditor");
	theme->voxelCfg.whiteEditor = assetManagerLoadTexture(whiteEditor, 0);

	const char* blackMiniEditor = jsonGetStr(json, "blackMiniEditor");
	theme->voxelCfg.blackMiniEditor = assetManagerLoadTexture(blackMiniEditor, 0);

	const char* whiteMiniEditor = jsonGetStr(json, "whiteMiniEditor");
	theme->voxelCfg.whiteMiniEditor = assetManagerLoadTexture(whiteMiniEditor, 0);
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
	theme->antlineCfg.editorAntlineTex = assetManagerLoadTexture(antlineTex, 0);
	const char* antlineActiveTex = jsonGetStr(json, "editorAntlineActiveTex");
	theme->antlineCfg.editorAntlineActiveTex = assetManagerLoadTexture(antlineActiveTex, 0);

	const char* checkTex = jsonGetStr(json, "editorCheckTex");
	theme->antlineCfg.editorCheckTex = assetManagerLoadTexture(checkTex, 0);
	const char* checkActiveTex = jsonGetStr(json, "editorCheckActiveTex");
	theme->antlineCfg.editorCheckActiveTex = assetManagerLoadTexture(checkActiveTex, 0);

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

		cJSON* tonemap = jsonGetObject(item, "tonemap");
		data->enableTonemap = jsonGetBoolC(tonemap, "enabled", 1);
		data->maxExposure = jsonGetFloat(tonemap, "maxExposure");
		data->minExposure = jsonGetFloat(tonemap, "minExposure");
		data->tonemapRate = jsonGetFloat(tonemap, "tonemapRate");
		data->brightPixelPercent = jsonGetFloat(tonemap, "brightPixelPercent");
		popStackFrame();

		cJSON* fog = jsonGetObject(item, "fog");
		data->enableFog = jsonGetBoolC(fog, "enabled", 1);
		jsonGetVec3(fog, "primaryColor", data->primaryFogColor);
		jsonGetVec3(fog, "secondaryColor", data->secondaryFogColor);
		data->primaryFogColor[0] /= 255.0f;
		data->primaryFogColor[1] /= 255.0f;
		data->primaryFogColor[2] /= 255.0f;
		data->secondaryFogColor[0] /= 255.0f;
		data->secondaryFogColor[1] /= 255.0f;
		data->secondaryFogColor[2] /= 255.0f;
		data->fogStart = jsonGetFloat(fog, "start");
		data->fogEnd = jsonGetFloat(fog, "end");
		data->fogDensity = jsonGetFloat(fog, "density");
		popStackFrame();

		cJSON* colorCorrection = jsonGetObject(item, "colorCorrection");
		data->enableColorCorrection = jsonGetBoolC(colorCorrection, "enabled", 1);
		data->minFalloff = jsonGetFloat(colorCorrection, "minFalloff");
		data->maxFalloff = jsonGetFloat(colorCorrection, "maxFalloff");
		data->maxWeight = jsonGetFloat(colorCorrection, "maxWeight");
		data->fadeInDuration = jsonGetFloat(colorCorrection, "fadeInDuration");
		data->fadeOutDuration = jsonGetFloat(colorCorrection, "fadeOutDuration");
		data->filename = jsonGetStrD(colorCorrection, "filename");
		popStackFrame();

		popStackFrame();
	}
	if (arrSize == 0)
		errorf("no post processing presets set");
	dynList_trim((void**)&theme->postProcessPresets);
}

void loadTheme(const char* filename)
{
	char path[64];
	printf("[package loader] loading theme %s\n", filename);
	snprintf(path, 64, "themes/%s", filename);

	int res = 0;
	initErrorStack(strdupf("__%s__/themes/%s", currentPackage->name, filename), res);
	if (res)
		startErrorLoop();

	int dataSize;
	char* data = readPackageFile(currentPackage, path, &dataSize);
	data[dataSize] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);
	if (err)
		jsonParseError(data, err, path);

	int themeCount = dynList_size(themeList);
	dynList_resize((void**)&themeList, themeCount + 1);
	Theme* theme = &themeList[themeCount];
	cJSON* voxel = jsonGetObject(json, "voxel");
	loadVoxelConfig(theme, voxel);
	popStackFrame();
	cJSON* antline = jsonGetObject(json, "antline");
	loadAntlineConfig(theme, antline);
	popStackFrame();
	cJSON* postProcessing = jsonGetArray(json, "postProcessing");
	loadPostProcessingPresets(theme, postProcessing);
	popStackFrame();

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
	char* filename = mapSettings.postProcess.filename;
	memcpy(&mapSettings.postProcess, &theme->postProcessPresets[0], sizeof(PostProcessData));

	mapSettings.postProcessPreset = 0;

	mapSettings.postProcess.filename = filename;
	strncpy(filename, theme->postProcessPresets[0].filename, 256);
}
