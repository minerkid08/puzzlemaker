#include "voxelConfig.h"
#include "assetManager.h"
#include "cjson.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

VoxelConfig voxelConfig;

static char* loadField(cJSON* json, const char* name)
{
	cJSON* item = cJSON_GetObjectItem(json, name);
	if (item == 0)
		errorf("undefined field %s in voxel.json\n", name);
	const char* v = cJSON_GetStringValue(item);
	return strdup(v);
}

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
		errorf("failed to parse voxel.json\n%s\n", err);

	voxelConfig.nodraw = loadField(json, "nodraw");
	voxelConfig.blackFloor = loadField(json, "blackFloor");
	voxelConfig.blackWall = loadField(json, "blackWall");
	voxelConfig.blackCeiling = loadField(json, "blackCeiling");
	voxelConfig.whiteFloor = loadField(json, "whiteFLoor");
	voxelConfig.whiteWall = loadField(json, "whiteWall");
	voxelConfig.whiteCeiling = loadField(json, "whiteCeiling");

	char* blackEditor = loadField(json, "blackEditor");
	voxelConfig.blackEditor = assetManagerLoadTexture(blackEditor);

	char* whiteEditor = loadField(json, "whiteEditor");
	voxelConfig.whiteEditor = assetManagerLoadTexture(whiteEditor);
  free(blackEditor);
  free(whiteEditor);

  cJSON_free(json);
}
