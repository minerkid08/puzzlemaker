#include "jsonUtils.h"
#include "assetManager.h"
#include "cjson.h"
#include "error.h"
#include "renderer/texture.h"
#include "ui/itemPanel.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void jsonParseError(const char* data, const char* inErr, const char* filename)
{
	int offset = inErr - data;
	int line = 0;
	int lineIndex = 0;
	for (int i = 0; i < offset; i++)
	{
		if (data[i] == '\n')
		{
			line++;
			lineIndex = i + 1;
		}
	}
	int col = offset - lineIndex;
	char buf[512];
	snprintf(buf, 512, "failed to parse %s, line %d col %d", filename, line, col);
	errorf(buf);
}

cJSON* jsonGetObject(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		errorf("unknown key %s", name);
	if (!cJSON_IsObject(arr))
		errorf("bad type for key %s, expected object", name);

	pushStackFrame(name);
	return arr;
}

cJSON* jsonGetArray(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		errorf("unknown key %s", name);
	if (!cJSON_IsArray(arr))
		errorf("bad type for key %s, expected array", name);

	pushStackFrame(name);
	return arr;
}

cJSON* jsonArrGetObject(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		errorf("array index %d is null", i);
	if (!cJSON_IsObject(arr))
		errorf("bad type for array index %d, expected object", i);

	char buf[64];
	snprintf(buf, 64, "index %d", i);
	pushStackFrame(buf);
	return arr;
}

cJSON* jsonArrGetArray(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		errorf("array index %d is null", i);
	if (!cJSON_IsArray(arr))
		errorf("bad type for array index %d, expected array", i);

	char buf[64];
	snprintf(buf, 64, "index %d", i);
	pushStackFrame(buf);
	return arr;
}

cJSON* jsonGetObjectC(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		return 0;
	if (!cJSON_IsObject(arr))
		errorf("bad type for key %s, expected object", name);

	pushStackFrame(name);
	return arr;
}

cJSON* jsonGetArrayC(const cJSON* json, const char* name)
{
	cJSON* arr = cJSON_GetObjectItem(json, name);
	if (arr == 0)
		return 0;
	if (!cJSON_IsArray(arr))
		errorf("bad type for key %s, expected array", name);

	pushStackFrame(name);
	return arr;
}

cJSON* jsonArrGetObjectC(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		return 0;
	if (!cJSON_IsObject(arr))
		errorf("bad type for array index %d, expected object", i);

	char buf[64];
	snprintf(buf, 64, "index %d", i);
	pushStackFrame(buf);
	return arr;
}

cJSON* jsonArrGetArrayC(const cJSON* json, int i)
{
	cJSON* arr = cJSON_GetArrayItem(json, i);
	if (arr == 0)
		return 0;
	if (!cJSON_IsArray(arr))
		errorf("bad type for array index %d, expected array", i);

	char buf[64];
	snprintf(buf, 64, "index %d", i);
	pushStackFrame(buf);
	return arr;
}

const char* jsonGetStr(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsString(j))
		errorf("wrong type for key '%s', expected string", name);

	return j->valuestring;
}

char* jsonGetStrD(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsString(j))
		errorf("wrong type for key '%s', expected string", name);

	return strdup(j->valuestring);
}

char jsonGetBool(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsBool(j))
		errorf("wrong type for key '%s', expected bool", name);
	return j->type == cJSON_True;
}

float jsonGetFloat(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsNumber(j))
		errorf("wrong type for key '%s', expected number", name);
	return j->valuedouble;
}

int jsonGetInt(const cJSON* json, const char* name)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsNumber(j))
		errorf("wrong type for key '%s', expected number", name);
	return j->valuedouble;
}

void jsonGetVec2(const cJSON* json, const char* name, vec2 out)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsArray(j))
		errorf("wrong type for key '%s', expected array", name);
	out[0] = jsonArrGetFloat(j, 0);
	out[1] = jsonArrGetFloat(j, 1);
}

void jsonGetVec3(const cJSON* json, const char* name, vec3 out)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		errorf("unknown key '%s'", name);
	if (!cJSON_IsArray(j))
		errorf("wrong type for key '%s', expected array", name);
	out[0] = jsonArrGetFloat(j, 0);
	out[1] = jsonArrGetFloat(j, 1);
	out[2] = jsonArrGetFloat(j, 2);
}

void jsonGetTex(const cJSON* json, const char* name, Texture* out)
{
	cJSON* tex = cJSON_GetObjectItem(json, name);
	out->alignment = TEX_ALIGNMENT_FIT;
	out->size = 512;
	if (tex == 0)
		errorf("unknown key '%s'", name);
	if (cJSON_IsString(tex))
	{
		out->filename = strdup(tex->valuestring);
		return;
	}

	pushStackFrame(name);

	out->filename = jsonGetStrD(tex, "filename");
	const char* alignment = jsonGetStrC(tex, "alignment", 0);
	if (alignment)
	{
		if (strcmp(alignment, "none") == 0)
			out->alignment = TEX_ALIGNMENT_NONE;
		else if (strcmp(alignment, "fit") == 0)
			out->alignment = TEX_ALIGNMENT_FIT;
		else
			errorf("invalid alignment mode");
	}
	out->size = jsonGetFloatC(tex, "size", 512);
	popStackFrame();
}

void jsonGetEditorTex(const cJSON* json, const char* name, Texture* out, char* flags)
{
	cJSON* tex = cJSON_GetObjectItem(json, name);
	out->alignment = TEX_ALIGNMENT_FIT;
	out->size = 1;
	if (tex == 0)
		errorf("unknown key '%s'", name);

	if (cJSON_IsString(tex))
	{
		out->tex = assetManagerLoadTexture(tex->valuestring, flags);
		return;
	}

	pushStackFrame(name);

	out->filename = jsonGetStr(tex, "filename");
	out->tex = assetManagerLoadTexture(out->filename, flags);
	const char* alignment = jsonGetStrC(tex, "alignment", 0);
	if (alignment)
	{
		if (strcmp(alignment, "none") == 0)
			out->alignment = TEX_ALIGNMENT_NONE;
		else if (strcmp(alignment, "fit") == 0)
			out->alignment = TEX_ALIGNMENT_FIT;
		else
			errorf("invalid alignment mode");
	}
	out->size = jsonGetFloatC(tex, "size", 1);
	popStackFrame();
}

void jsonGetTransform(const cJSON* json, const char* name, vec3 pos, vec3 rot, mat4 mat)
{
	cJSON* transform = jsonGetObjectC(json, name);
	if (transform)
	{
		pos[0] = 0;
		pos[1] = 0;
		pos[2] = 0;

		rot[0] = 0;
		rot[1] = 0;
		rot[2] = 0;

		cJSON* position = cJSON_GetObjectItem(transform, "position");
		cJSON* rotation = cJSON_GetObjectItem(transform, "rotation");
		if (position)
			jsonGetVec3(transform, "position", pos);
		if (rotation)
			jsonGetVec3(transform, "rotation", rot);
		popStackFrame();
	}
	if (mat)
	{
		vec3 r2;
		glm_mat4_identity(mat);
		mat4 rotMat;
		r2[0] = glm_rad(rot[0]);
		r2[1] = glm_rad(rot[1]);
		r2[2] = glm_rad(rot[2]);
		glm_translate(mat, pos);
		glm_euler_yxz(r2, rotMat);
		glm_mat4_mul(mat, rotMat, mat);
	}
}

const char* jsonGetStrC(const cJSON* json, const char* name, char* def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsString(j))
		errorf("wrong type for key '%s', expected string", name);

	return j->valuestring;
}

char* jsonGetStrCD(const cJSON* json, const char* name, char* def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsString(j))
		errorf("wrong type for key '%s', expected string", name);

	return strdup(j->valuestring);
}

char jsonGetBoolC(const cJSON* json, const char* name, char def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsBool(j))
		errorf("wrong type for key '%s', expected bool", name);
	return j->type == cJSON_True;
}

float jsonGetFloatC(const cJSON* json, const char* name, float def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsNumber(j))
		errorf("wrong type for key '%s', expected number", name);
	return j->valuedouble;
}

int jsonGetIntC(const cJSON* json, const char* name, int def)
{
	cJSON* j = cJSON_GetObjectItem(json, name);
	if (j == 0)
		return def;
	if (!cJSON_IsNumber(j))
		errorf("wrong type for key '%s', expected number", name);
	return j->valuedouble;
}

char* jsonArrGetStr(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		errorf("unknown arr index '%d'", i);
	if (!cJSON_IsString(j))
		errorf("wrong type for arr elem '%d', expected string", i);
	int len = strlen(j->valuestring);

	char* buf = malloc(len + 1);
	strcpy(buf, j->valuestring);
	return buf;
}

char jsonArrGetBool(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		errorf("unknown arr index '%d'", i);
	if (!cJSON_IsBool(j))
		errorf("wrong type for arr elem '%d', expected string", i);
	return j->type == cJSON_True;
}

float jsonArrGetFloat(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		errorf("unknown arr index '%d'", i);
	if (!cJSON_IsNumber(j))
		errorf("wrong type for arr elem '%d', expected number", i);
	return j->valuedouble;
}

int jsonArrGetInt(const cJSON* json, int i)
{
	cJSON* j = cJSON_GetArrayItem(json, i);
	if (j == 0)
		errorf("unknown arr index '%d'", i);
	if (!cJSON_IsNumber(j))
		errorf("wrong type for arr elem '%d', expected number", i);
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
