#include "item/item.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include "utils.h"
#include <stdio.h>
#include <dynList.h>
#include "cjson.h"
#include "jsonUtils.h"
#include <string.h>

extern ItemDefinition* definitions;
extern Item* items;

void initItems()
{
	items = dynList_new(0, sizeof(Item));

	FILE* file = fopen("items.json", "rb");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);

	int l = cJSON_GetArraySize(json);
	definitions = dynList_new(l, sizeof(ItemDefinition));

	int i = 0;
	cJSON* item;
	cJSON_ArrayForEach(item, json)
	{
		ItemDefinition* def = &definitions[i++];

		def->name = jsonGetStr(item, "name");
		def->modelName = jsonGetStr(item, "model");
		def->textureName = jsonGetStr(item, "mat");
		if (cJSON_HasObjectItem(item, "instance"))
		{
			def->type = ENTITY_TYPE_INSTANCE;
			def->instanceName = jsonGetStr(item, "instance");
		}
		else
		{
			def->type = ENTITY_TYPE_ENTITY;
			def->instanceName = jsonGetStr(item, "entity");
		}

		cJSON* keyValues = cJSON_GetObjectItem(item, "keyvalues");
		len = cJSON_GetArraySize(keyValues);
		def->kvs = dynList_new(len, sizeof(ItemKvDef));

		for (int i = 0; i < len; i++)
		{
			cJSON* kv = cJSON_GetArrayItem(keyValues, i);
			ItemKvDef* kvDef = &def->kvs[i];

			kvDef->name = jsonGetStr(kv, "name");

			cJSON* type = cJSON_GetObjectItem(kv, "type");
			if (strcmp(type->valuestring, "bool") == 0)
			{
				kvDef->type = TYPE_BOOL;
				kvDef->defaultValue.b = jsonGetBool(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "int") == 0)
			{
				kvDef->type = TYPE_INT;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "drop-string") == 0)
			{
				kvDef->type = TYPE_STRING | TYPE_DROPDOWN;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
				cJSON* options = cJSON_GetObjectItem(kv, "options");
				cJSON* opt = options->child;
				int len = cJSON_GetArraySize(options);

				kvDef->dropNames = dynList_new(len, sizeof(char*));
				kvDef->dropValues = dynList_new(len, sizeof(V));
				int i = 0;
				while (1)
				{
					if (opt == 0)
						break;
					kvDef->dropNames[i] = copyString(opt->string);
					kvDef->dropValues[i].s = copyString(opt->valuestring);
					opt = opt->next;
					i++;
				}
			}
			if (strcmp(type->valuestring, "drop-int") == 0)
			{
				kvDef->type = TYPE_INT | TYPE_DROPDOWN;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
				cJSON* options = cJSON_GetObjectItem(kv, "options");
				cJSON* opt = options->child;
				int len = cJSON_GetArraySize(options);

				kvDef->dropNames = dynList_new(len, sizeof(char*));
				kvDef->dropValues = dynList_new(len, sizeof(V));
				int i = 0;
				while (1)
				{
					if (opt == 0)
						break;
					kvDef->dropNames[i] = copyString(opt->string);
					kvDef->dropValues[i].i = opt->valuedouble;
					opt = opt->next;
					i++;
				}
			}
		}

		cJSON* inputs = cJSON_GetObjectItem(item, "inputs");
		len = cJSON_GetArraySize(inputs);
		def->inputs = dynList_new(len, sizeof(InputDef));
		for (int i = 0; i < len; i++)
		{
			cJSON* input = cJSON_GetArrayItem(inputs, i);
			InputDef* inputDef = &def->inputs[i];

			inputDef->name = jsonGetStr(input, "name");
			inputDef->trueInput = jsonGetStr(input, "trueInput");
			inputDef->falseInput = jsonGetStr(input, "falseInput");
			inputDef->trueArg = 0;
			inputDef->falseArg = 0;
			if (cJSON_GetObjectItem(input, "trueArg"))
				inputDef->trueArg = jsonGetStr(input, "trueArg");
			if (cJSON_GetObjectItem(input, "falseArg"))
				inputDef->falseArg = jsonGetStr(input, "falseArg");
		}

		cJSON* outputs = cJSON_GetObjectItem(item, "outputs");
		len = cJSON_GetArraySize(outputs);
		def->outputs = dynList_new(len, sizeof(OutputDef));
		for (int i = 0; i < len; i++)
		{
			cJSON* output = cJSON_GetArrayItem(outputs, i);
			OutputDef* outputDef = &def->outputs[i];

			outputDef->name = jsonGetStr(output, "name");
			outputDef->trueOutput = jsonGetStr(output, "trueOutput");
			outputDef->falseOutput = jsonGetStr(output, "falseOutput");
		}

		cJSON* bound = cJSON_GetObjectItem(item, "bound1");
		def->bound1[0] = jsonArrGetFloat(bound, 0);
		def->bound1[1] = jsonArrGetFloat(bound, 1);
		def->bound1[2] = jsonArrGetFloat(bound, 2);
		def->bound1[3] = 1;

		bound = cJSON_GetObjectItem(item, "bound2");
		def->bound2[0] = jsonArrGetFloat(bound, 0);
		def->bound2[1] = jsonArrGetFloat(bound, 1);
		def->bound2[2] = jsonArrGetFloat(bound, 2);
		def->bound2[3] = 1;

		def->mesh = loadMesh(def->modelName);
		def->texture = loadTexture(def->textureName);

		cJSON* staticKvs = cJSON_GetObjectItem(item, "statickvs");
		int len = cJSON_GetArraySize(staticKvs);

		def->staticKvs = dynList_new(len, sizeof(char*));
		if (staticKvs)
		{
			cJSON* kv = staticKvs->child;
			int i = 0;
			while (1)
			{
				if (kv == 0)
					break;
				int l = strlen(kv->string) + strlen(kv->valuestring) + 7;
				def->staticKvs[i] = malloc(l);
				snprintf(def->staticKvs[i], l, "\"%s\" \"%s\"", kv->string, kv->valuestring);
				kv = kv->next;
				i++;
			}
		}

		cJSON* offset = cJSON_GetObjectItem(item, "offset");
		if (offset)
		{
			def->offset[0] = jsonArrGetFloat(offset, 0);
			def->offset[1] = jsonArrGetFloat(offset, 1);
			def->offset[2] = jsonArrGetFloat(offset, 2);
		}
		else
		{
			def->offset[0] = 0;
			def->offset[1] = 0;
			def->offset[2] = 0;
		}
	}

	err = cJSON_GetErrorPtr();
	if (err)
	{
		printf("json error\n%s\n", err);
		exit(1);
	}
	cJSON_free(json);
}
