#include "jsonUtils.h"
#include "cjson.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

char* jsonGetStr(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
    errorf("unknown key '%s'\n", name);
	if(!cJSON_IsString(j))
    errorf("wrong type for key '%s', expected string\n", name);
	
	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonGetBool(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
    errorf("unknown key '%s'\n", name);
	if(!cJSON_IsBool(j))
    errorf("wrong type for key '%s', expected bool\n", name);
	return j->type == cJSON_True;
}

float jsonGetFloat(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
    errorf("unknown key '%s'\n", name);
	if(!cJSON_IsNumber(j))
    errorf("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

int jsonGetInt(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
    errorf("unknown key '%s'\n", name);
	if(!cJSON_IsNumber(j))
    errorf("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

void jsonGetVec2(const cJSON* json, const char* name, vec2 out)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
    errorf("unknown key '%s'\n", name);
	if(!cJSON_IsArray(j))
    errorf("wrong type for key '%s', expected array\n", name);
	out[0] = jsonArrGetFloat(j, 0);
	out[1] = jsonArrGetFloat(j, 1);
}

void jsonGetVec3(const cJSON* json, const char* name, vec3 out)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
    errorf("unknown key '%s'\n", name);
	if(!cJSON_IsArray(j))
    errorf("wrong type for key '%s', expected array\n", name);
	out[0] = jsonArrGetFloat(j, 0);
	out[1] = jsonArrGetFloat(j, 1);
	out[2] = jsonArrGetFloat(j, 2);
}

char jsonGetBoolC(const cJSON* json, const char* name, char def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
		return def;
	if(!cJSON_IsBool(j))
		return def;
	return j->type == cJSON_True;
}

float jsonGetFloatC(const cJSON* json, const char* name, float def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
  if(j == 0)
		return def;
	if(!cJSON_IsNumber(j))
    errorf("wrong type for key '%s', expected number\n", name);
	return j->valuedouble;
}

char* jsonArrGetStr(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
  if(j == 0)
    errorf("unknown arr index '%d'\n", i);
	if(!cJSON_IsString(j))
    errorf("wrong type for arr elem '%d', expected string\n", i);
	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonArrGetBool(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
  if(j == 0)
    errorf("unknown arr index '%d'\n", i);
	if(!cJSON_IsBool(j))
    errorf("wrong type for arr elem '%d', expected string\n", i);
	return j->type == cJSON_True;
}

float jsonArrGetFloat(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
  if(j == 0)
    errorf("unknown arr index '%d'\n", i);
	if(!cJSON_IsNumber(j))
    errorf("wrong type for arr elem '%d', expected number\n", i);
	return j->valuedouble;
}

int jsonArrGetInt(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
  if(j == 0)
    errorf("unknown arr index '%d'\n", i);
	if(!cJSON_IsNumber(j))
    errorf("wrong type for arr elem '%d', expected number\n", i);
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
