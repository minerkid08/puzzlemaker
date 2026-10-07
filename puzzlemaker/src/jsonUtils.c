#include "jsonUtils.h"
#include "cjson.h"
#include "dynList.h"
#include "ui/itemPanel.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char** stackTrace;
extern char* err;
extern char errorState;

static char** jsonStack;

static __attribute__((constructor)) void init()
{
	jsonStack = dynList_new(0, sizeof(const char*));
}

void jsonParseError(const char* data, const char* inErr, const char* filename)
{
	int offset = inErr - data;
	int line = 0;
	int lineIndex = 0;
	for(int i = 0; i < offset; i++)
	{
		if(data[i] == '\n')
		{
			line++;
			lineIndex = i + 1;
		}
	}
	int col = offset - lineIndex;
	char buf[512];
	snprintf(buf, 512, "failed to parse %s, line %d col %d\n", filename, line, col);
	err = strdup(buf);
	errorState = 1;
	startErrorLoop();
}

void jsonResetStack(const char* name)
{
	int len = dynList_size(jsonStack);
	for (int i = 0; i < len; i++)
		free(jsonStack[i]);
	dynList_resize((void**)&jsonStack, 1);
	jsonStack[0] = strdup(name);
}

void jsonError(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	char buf[256];
	vsnprintf(buf, 256, fmt, va);
	printf("%s", buf);
	int len = dynList_size(jsonStack);
	for (int i = 0; i < len; i++)
		printf("in %s\n", jsonStack[i]);

	stackTrace = jsonStack;
	err = strdup(buf);
	errorState = 1;
	startErrorLoop();
}

cJSON* jsonGetObject(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		jsonError("unknown key %s\n", name);
	if (!cJSON_IsObject(arr))
		jsonError("bad type for key %s, expected object\n", name);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	jsonStack[len] = strdup(name);
	return arr;
}

cJSON* jsonGetArray(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		jsonError("unknown key %s\n", name);
	if (!cJSON_IsArray(arr))
		jsonError("bad type for key %s, expected array\n", name);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	jsonStack[len] = strdup(name);
	return arr;
}

cJSON* jsonArrGetObject(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		jsonError("array index %d is null\n", i);
	if (!cJSON_IsObject(arr))
		jsonError("bad type for array index %d, expected object\n", i);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	char buf[64];
	snprintf(buf, 64, "index %d", i);
	jsonStack[len] = strdup(buf);
	return arr;
}

cJSON* jsonArrGetArray(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		jsonError("array index %d is null\n", i);
	if (!cJSON_IsArray(arr))
		jsonError("bad type for array index %d, expected array\n", i);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	char buf[64];
	snprintf(buf, 64, "index %d", i);
	jsonStack[len] = strdup(buf);
	return arr;
}

cJSON* jsonGetObjectC(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		return 0;
	if (!cJSON_IsObject(arr))
		jsonError("bad type for key %s, expected object\n", name);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	jsonStack[len] = strdup(name);
	return arr;
}

cJSON* jsonGetArrayC(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		return 0;
	if (!cJSON_IsArray(arr))
		jsonError("bad type for key %s, expected array\n", name);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	jsonStack[len] = strdup(name);
	return arr;
}

cJSON* jsonArrGetObjectC(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		return 0;
	if (!cJSON_IsObject(arr))
		jsonError("bad type for array index %d, expected object\n", i);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	char buf[64];
	snprintf(buf, 64, "index %d", i);
	jsonStack[len] = strdup(buf);
	return arr;
}

cJSON* jsonArrGetArrayC(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		return 0;
	if (!cJSON_IsArray(arr))
		jsonError("bad type for array index %d, expected array\n", i);

	int len = dynList_size(jsonStack);
	dynList_resize((void**)&jsonStack, len + 1);
	char buf[64];
	snprintf(buf, 64, "index %d", i);
	jsonStack[len] = strdup(buf);
	return arr;
}

void jsonPop()
{
	int len = dynList_size(jsonStack);
	free(jsonStack[len - 1]);
	dynList_resize((void**)&jsonStack, len - 1);
}

char* jsonGetStr(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		jsonError("unknown key '%s'\n", name);
	if (!cJSON_IsString(j))
		jsonError("wrong type for key '%s', expected string\n", name);

	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonGetBool(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		jsonError("unknown key '%s'\n", name);
	if (!cJSON_IsBool(j))
		jsonError("wrong type for key '%s', expected bool\n", name);
	return j->type == cJSON_True;
}

float jsonGetFloat(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		jsonError("unknown key '%s'\n", name);
	if (!cJSON_IsNumber(j))
		jsonError("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

int jsonGetInt(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		jsonError("unknown key '%s'\n", name);
	if (!cJSON_IsNumber(j))
		jsonError("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

void jsonGetVec2(const cJSON* json, const char* name, vec2 out)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		jsonError("unknown key '%s'\n", name);
	if (!cJSON_IsArray(j))
		jsonError("wrong type for key '%s', expected array\n", name);
	out[0] = jsonArrGetFloat(j, 0);
	out[1] = jsonArrGetFloat(j, 1);
}

void jsonGetVec3(const cJSON* json, const char* name, vec3 out)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		jsonError("unknown key '%s'\n", name);
	if (!cJSON_IsArray(j))
		jsonError("wrong type for key '%s', expected array\n", name);
	out[0] = jsonArrGetFloat(j, 0);
	out[1] = jsonArrGetFloat(j, 1);
	out[2] = jsonArrGetFloat(j, 2);
}

char* jsonGetStrC(const cJSON* json, const char* name, char* def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsString(j))
		jsonError("wrong type for key '%s', expected string\n", name);

	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonGetBoolC(const cJSON* json, const char* name, char def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsBool(j))
		jsonError("wrong type for key '%s', expected bool\n", name);
	return j->type == cJSON_True;
}

float jsonGetFloatC(const cJSON* json, const char* name, float def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsNumber(j))
		jsonError("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

int jsonGetIntC(const cJSON* json, const char* name, int def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsNumber(j))
		jsonError("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

char* jsonArrGetStr(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		jsonError("unknown arr index '%d'\n", i);
	if (!cJSON_IsString(j))
		jsonError("wrong type for arr elem '%d', expected string\n", i);
	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonArrGetBool(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		jsonError("unknown arr index '%d'\n", i);
	if (!cJSON_IsBool(j))
		jsonError("wrong type for arr elem '%d', expected string\n", i);
	return j->type == cJSON_True;
}

float jsonArrGetFloat(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		jsonError("unknown arr index '%d'\n", i);
	if (!cJSON_IsNumber(j))
		jsonError("wrong type for arr elem '%d', expected number\n", i);
	return j->valuedouble;
}

int jsonArrGetInt(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		jsonError("unknown arr index '%d'\n", i);
	if (!cJSON_IsNumber(j))
		jsonError("wrong type for arr elem '%d', expected number\n", i);
	return j->valuedouble;
}

void jsonArrSetStr(cJSON* arr, const char* s)
{
	cJSON* j = cJSON_CreateString(s);
	cJSON_AddItemToArray(arr, j);
}

void jsonArrSetBool(cJSON* arr, char b)
{
	cJSON* j = cJSON_CreateBool(b);
	cJSON_AddItemToArray(arr, j);
}

void jsonArrSetFloat(cJSON* arr, float f)
{
	cJSON* j = cJSON_CreateNumber(f);
	cJSON_AddItemToArray(arr, j);
}

void jsonArrSetInt(cJSON* arr, int i)
{
	cJSON* j = cJSON_CreateNumber(i);
	cJSON_AddItemToArray(arr, j);
}

void jsonSetVec3(cJSON* json, const char* name, vec3 vec)
{
	cJSON* j = cJSON_CreateArray();
	cJSON* x = cJSON_CreateNumber(vec[0]);
	cJSON* y = cJSON_CreateNumber(vec[1]);
	cJSON* z = cJSON_CreateNumber(vec[2]);

	cJSON_AddItemToArray(j, x);
	cJSON_AddItemToArray(j, y);
	cJSON_AddItemToArray(j, z);
	cJSON_AddItemToObject(json, name, j);
}
