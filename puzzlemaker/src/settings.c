#include "settings.h"
#include "cjson.h"
#include "error.h"
#include "utils.h"
#include "jsonUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EditorSettings editorSettings;

void loadEditorSettings()
{
	FILE* file = fopen("editorSettings.json", "rb");
	if (file == 0)
	{
		editorSettings.boostSpeed = 8;
		editorSettings.moveSpeed = 4;
		editorSettings.fov = 90;
		editorSettings.rotSnap = 90;
		return;
	}

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	int res;
	initErrorStackD("editorSettings.json", res);
	if(res)
		startErrorLoop();

	cJSON* json = cJSON_Parse(data);
	if (json == 0)
		jsonParseError(data, cJSON_GetErrorPtr(), "editorSettings.json");
	free(data);

	editorSettings.moveSpeed = jsonGetFloat(json, "moveSpeed");
	editorSettings.boostSpeed = jsonGetFloat(json, "boostSpeed");
	editorSettings.fov = jsonGetFloat(json, "fov");
	editorSettings.rotSnap = jsonGetFloatC(json, "rotSnap", 90);
	cJSON_Delete(json);
}

void saveEditorSettings()
{
	cJSON* json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "moveSpeed", editorSettings.moveSpeed);
	cJSON_AddNumberToObject(json, "boostSpeed", editorSettings.boostSpeed);
	cJSON_AddNumberToObject(json, "fov", editorSettings.fov);
	cJSON_AddNumberToObject(json, "rotSnap", editorSettings.rotSnap);

	char* str = cJSON_Print(json);

	FILE* file = fopen("editorSettings.json", "wb");
	fwrite(str, strlen(str), 1, file);
	fclose(file);

	free(str);
  cJSON_Delete(json);
}
