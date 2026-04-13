#include "jsonUtils.h"
#include "cjson.h"

#include <stdlib.h>
#include <string.h>

const char* jsonGetStr(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonGetBool(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	return j->type == cJSON_True;
}

float jsonGetFloat(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	return j->valuedouble;
}

int jsonGetInt(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	return j->valuedouble;
}

const char* jsonArrGetStr(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonArrGetBool(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	return j->type == cJSON_True;
}

float jsonArrGetFloat(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	return j->valuedouble;
}

int jsonArrGetInt(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
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
