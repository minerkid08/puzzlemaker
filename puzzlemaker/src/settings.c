#include "settings.h"
#include "cjson.h"
#include "error.h"
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
		editorSettings.arrowSens = 1;
		editorSettings.orbitSens = 0.1;
		editorSettings.panSens = 0.5;
		editorSettings.fov = 90;
		editorSettings.rotSnap = 90;
		editorSettings.lockMouseOnPan = 1;
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
	if (res)
		startErrorLoop();

	cJSON* json = cJSON_Parse(data);
	if (json == 0)
		jsonParseError(data, cJSON_GetErrorPtr(), "editorSettings.json");
	free(data);

	editorSettings.moveSpeed = jsonGetFloatC(json, "moveSpeed", 4);
	editorSettings.boostSpeed = jsonGetFloatC(json, "boostSpeed", 8);
	editorSettings.arrowSens = jsonGetFloatC(json, "arrowSens", 1);
	editorSettings.orbitSens = jsonGetFloatC(json, "orbitSens", 0.1);
	editorSettings.panSens = jsonGetFloatC(json, "panSens", 0.5);
	editorSettings.fov = jsonGetFloatC(json, "fov", 90);
	editorSettings.rotSnap = jsonGetFloatC(json, "rotSnap", 90);
	editorSettings.lockMouseOnPan = jsonGetBoolC(json, "lockMouseOnPan", 1);
	cJSON_Delete(json);
}

void saveEditorSettings()
{
	cJSON* json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "moveSpeed", editorSettings.moveSpeed);
	cJSON_AddNumberToObject(json, "boostSpeed", editorSettings.boostSpeed);
	cJSON_AddNumberToObject(json, "arrowSens", editorSettings.arrowSens);
	cJSON_AddNumberToObject(json, "orbitSens", editorSettings.orbitSens);
	cJSON_AddNumberToObject(json, "panSens", editorSettings.panSens);
	cJSON_AddNumberToObject(json, "fov", editorSettings.fov);
	cJSON_AddNumberToObject(json, "rotSnap", editorSettings.rotSnap);
	cJSON_AddBoolToObject(json, "lockMouseOnPan", editorSettings.lockMouseOnPan);

	char* str = cJSON_Print(json);

	FILE* file = fopen("editorSettings.json", "wb");
	fwrite(str, strlen(str), 1, file);
	fclose(file);

	free(str);
	cJSON_Delete(json);
}
