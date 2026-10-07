#include "voxelConfig.h"
#include "assetManager.h"
#include "cjson.h"
#include "utils.h"
#include "jsonUtils.h"
#include <stdio.h>

VoxelConfig voxelConfig;

void loadVoxelConfig()
{
	FILE* file = fopen("voxel.json", "rb");
	if (file == 0)
		errorf("failed to open voxel.json\n");

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
		jsonParseError(data, err, "voxel.json");

	jsonResetStack("voxel.json");

	voxelConfig.nodraw = jsonGetStr(json, "nodraw");
	voxelConfig.backstage = jsonGetStr(json, "backstage");

	voxelConfig.blackFloor = jsonGetStr(json, "blackFloor");
	voxelConfig.blackWall = jsonGetStr(json, "blackWall");
	voxelConfig.blackCeiling = jsonGetStr(json, "blackCeiling");
	voxelConfig.whiteFloor = jsonGetStr(json, "whiteFloor");
	voxelConfig.whiteWall = jsonGetStr(json, "whiteWall");
	voxelConfig.whiteCeiling = jsonGetStr(json, "whiteCeiling");

	voxelConfig.blackFloorMini = jsonGetStr(json, "blackFloorMini");
	voxelConfig.blackWallMini = jsonGetStr(json, "blackWallMini");
	voxelConfig.blackCeilingMini = jsonGetStr(json, "blackCeilingMini");
	voxelConfig.whiteFloorMini = jsonGetStr(json, "whiteFloorMini");
	voxelConfig.whiteWallMini = jsonGetStr(json, "whiteWallMini");
	voxelConfig.whiteCeilingMini = jsonGetStr(json, "whiteCeilingMini");

	char* blackEditor = jsonGetStr(json, "blackEditor");
	voxelConfig.blackEditor = assetManagerLoadTexture(blackEditor);

	char* whiteEditor = jsonGetStr(json, "whiteEditor");
	voxelConfig.whiteEditor = assetManagerLoadTexture(whiteEditor);

	char* blackMiniEditor = jsonGetStr(json, "blackMiniEditor");
	voxelConfig.blackMiniEditor = assetManagerLoadTexture(blackMiniEditor);

	char* whiteMiniEditor = jsonGetStr(json, "whiteMiniEditor");
	voxelConfig.whiteMiniEditor = assetManagerLoadTexture(whiteMiniEditor);

	free(blackEditor);
	free(whiteEditor);
	free(blackMiniEditor);
	free(whiteMiniEditor);

	cJSON_Delete(json);
}
