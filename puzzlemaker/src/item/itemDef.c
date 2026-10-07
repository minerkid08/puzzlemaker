#include "cjson.h"
#include "dynList.h"
#include "item/entityItem.h"
#include "item/item.h"
#include "item/overlayItem.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "jsonUtils.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

ItemDefinition* definitions;
ItemGroup* groups;

ItemDefinition* getItemDefinitions()
{
	return definitions;
}

ItemGroup* getItemGroups()
{
	return groups;
}

void loadItemDefinitionFile(const char* filename);

void loadItemDefinitions()
{
	const char** files = dynList_new(0, sizeof(const char*));

	listFiles("items", 0, &files, ".json");

	definitions = dynList_new(0, sizeof(ItemDefinition));
	groups = dynList_new(0, sizeof(ItemGroup));

	int len = dynList_size(files);
	for (int i = 0; i < len; i++)
	{
		loadItemDefinitionFile(files[i]);
		free((void*)files[i]);
	}
	dynList_free(files);

	dynList_trim((void**)&definitions);
	dynList_trim((void**)&groups);
}

void loadItemDefinitionFile(const char* filename)
{
	char filenameBuf[64];
	snprintf(filenameBuf, 64, "items/%s", filename);
	printf("[item loader] loading file %s\n", filenameBuf);

	FILE* file = fopen(filenameBuf, "rb");

	strncpy(filenameBuf, filename, 64);
	for (int i = 0; i < strlen(filenameBuf); i++)
	{
		if (filenameBuf[i] == '.')
		{
			filenameBuf[i] = 0;
			break;
		}
	}
	const char* group = strdup(filenameBuf);

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();

	if(err)
		jsonParseError(data, err, filename);

	free(data);

	int l = cJSON_GetArraySize(json);
	int listLen = dynList_size(definitions);
	dynList_resize((void**)&definitions, l + listLen);

	int groupCount = dynList_size(groups);
	dynList_resize((void**)&groups, groupCount + 1);

	ItemGroup* itemGroup = &groups[groupCount];
	itemGroup->name = group;
	itemGroup->size = l;
	itemGroup->startInd = listLen;

	int i = listLen;
	cJSON* item;
	int j = 0;
	cJSON_ArrayForEach(item, json)
	{
		if (!cJSON_IsObject(item))
			errorf("bad type for item %s %d, expected object\n", filename, j);
		ItemDefinition* def = &definitions[i++];
		def->group = group;

		if (cJSON_GetObjectItem(item, "name") == 0)
			errorf("item %d is missing name field\n", i);

		def->name = jsonGetStr(item, "name");

		printf("[item loader] loading item '%s'\n", def->name);

		char buf[256];
		snprintf(buf, 256, "%s, %s index %d", def->name, filename, j++);
		jsonResetStack(buf);

		const char* type = jsonGetStr(item, "type");
		if (strcmp(type, "entity") == 0)
		{
			def->type = ITEM_TYPE_ENTITY;
			def->data = loadEntityItemDef(item, def);
		}
		else if (strcmp(type, "panel") == 0)
		{
			def->type = ITEM_TYPE_PANEL;
			def->data = loadPanelItemDef(item, def);
		}
		else if (strcmp(type, "volume") == 0)
		{
			def->type = ITEM_TYPE_VOLUME;
			def->data = loadVolumeItemDef(item, def);
		}
		else if (strcmp(type, "overlay") == 0)
		{
			def->type = ITEM_TYPE_OVERLAY;
			def->data = loadOverlayItemDef(item, def);
		}
		else
			jsonError("invalid value for key type type\n");

		def->deleteIntersectingVoxels = jsonGetBoolC(item, "deleteIntersectingVoxels", 0);
		def->genMissingVoxels = jsonGetBoolC(item, "genMissingVoxels", 1);
		def->exportScript = jsonGetStrC(item, "exportScript", 0);
		def->transparent = jsonGetBoolC(item, "transparent", 0);

		cJSON* keyValues = jsonGetArrayC(item, "keyvalues");
		len = cJSON_GetArraySize(keyValues);
		def->kvs = dynList_new(len, sizeof(ItemKvDef));

		for (int i = 0; i < len; i++)
		{
			cJSON* kv = jsonArrGetObject(keyValues, i);
			ItemKvDef* kvDef = &def->kvs[i];

			kvDef->name = jsonGetStr(kv, "kv");
			if (cJSON_GetObjectItem(kv, "name"))
				kvDef->displayName = jsonGetStr(kv, "name");
			else
				kvDef->displayName = kvDef->name;

			cJSON* type = cJSON_GetObjectItem(kv, "type");
			if (strcmp(type->valuestring, "bool") == 0)
			{
				kvDef->type = TYPE_BOOL;
				kvDef->defaultValue.b = jsonGetBool(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "float") == 0)
			{
				kvDef->type = TYPE_FLOAT;
				kvDef->defaultValue.f = jsonGetFloat(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "int") == 0)
			{
				kvDef->type = TYPE_INT;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "string") == 0)
			{
				kvDef->type = TYPE_STRING;
				kvDef->defaultValue.s = jsonGetStr(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "item") == 0)
			{
				kvDef->type = TYPE_PICKER;
				kvDef->defaultValue.i = -1;
			}
			if (strcmp(type->valuestring, "drop-string") == 0)
			{
				kvDef->type = TYPE_STRING | TYPE_DROPDOWN;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
				cJSON* options = jsonGetObject(kv, "options");
				cJSON* opt = options->child;
				int len = cJSON_GetArraySize(options);

				kvDef->dropNames = dynList_new(len, sizeof(char*));
				kvDef->dropValues = dynList_new(len, sizeof(V));
				int i = 0;
				while (1)
				{
					if (opt == 0)
						break;
					if (!cJSON_IsString(opt))
						jsonError("bad type for kv value %d\n", i);
					kvDef->dropNames[i] = strdup(opt->string);
					kvDef->dropValues[i].s = strdup(opt->valuestring);
					opt = opt->next;
					i++;
				}
				jsonPop();
			}
			if (strcmp(type->valuestring, "drop-int") == 0)
			{
				kvDef->type = TYPE_INT | TYPE_DROPDOWN;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
				cJSON* options = jsonGetObject(kv, "options");
				cJSON* opt = options->child;
				int len = cJSON_GetArraySize(options);

				kvDef->dropNames = dynList_new(len, sizeof(char*));
				kvDef->dropValues = dynList_new(len, sizeof(V));
				int i = 0;
				while (1)
				{
					if (opt == 0)
						break;
					if (!cJSON_IsNumber(opt))
						jsonError("bad type for kv value %d\n", i);
					kvDef->dropNames[i] = strdup(opt->string);
					kvDef->dropValues[i].i = opt->valuedouble;
					opt = opt->next;
					i++;
				}
				jsonPop();
			}

			if (def->type == ITEM_TYPE_ENTITY)
			{
				EntityItemDef* entDef = def->data;
				if (entDef->instanceName)
					kvDef->type |= TYPE_INSTANCE;
			}
			jsonPop();
		}
		if (len > 0)
			jsonPop();

		cJSON* inputs = jsonGetArrayC(item, "inputs");
		if (inputs == 0)
			def->inputs = dynList_new(0, sizeof(InputDef));
		else
		{
			len = cJSON_GetArraySize(inputs);
			def->inputs = dynList_new(len, sizeof(InputDef));
			for (int i = 0; i < len; i++)
			{
				cJSON* input = jsonArrGetObject(inputs, i);
				InputDef* inputDef = &def->inputs[i];

				inputDef->name = jsonGetStr(input, "name");
				inputDef->trueInput = jsonGetStr(input, "trueInput");
				inputDef->falseInput = jsonGetStrC(input, "falseInput", 0);
				inputDef->trueArg = jsonGetStrC(input, "trueArg", 0);
				inputDef->falseArg = jsonGetStrC(input, "falseArg", 0);
				jsonPop();
			}
			jsonPop();
		}

		cJSON* outputs = jsonGetArrayC(item, "outputs");
		if (outputs == 0)
			def->outputs = dynList_new(0, sizeof(OutputDef));
		else
		{
			len = cJSON_GetArraySize(outputs);
			def->outputs = dynList_new(len, sizeof(OutputDef));
			for (int i = 0; i < len; i++)
			{
				cJSON* output = jsonArrGetObject(outputs, i);
				OutputDef* outputDef = &def->outputs[i];

				outputDef->name = jsonGetStr(output, "name");
				outputDef->trueOutput = jsonGetStr(output, "trueOutput");
				outputDef->falseOutput = jsonGetStrC(output, "falseOutput", 0);
				jsonPop();
			}
			jsonPop();
		}

		cJSON* staticKvs = jsonGetObjectC(item, "statickvs");
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
				if (!cJSON_IsString(kv))
					jsonError("bad type for static kv value %d\n", i);
				int l = strlen(kv->string) + strlen(kv->valuestring) + 7;
				def->staticKvs[i] = malloc(l);
				snprintf(def->staticKvs[i], l, "\"%s\" \"%s\"", kv->string, kv->valuestring);
				kv = kv->next;
				i++;
			}
			jsonPop();
		}

		char* snapMode = jsonGetStrC(item, "snapMode", 0);
		def->snapMode = SNAP_CORNER;
		if (snapMode)
		{
			if (strcmp(snapMode, "corner") == 0)
				def->snapMode = SNAP_CORNER;
			if (strcmp(snapMode, "center") == 0)
				def->snapMode = SNAP_CENTER;
			if (strcmp(snapMode, "mini-corner") == 0)
				def->snapMode = SNAP_MINI_CORNER;
			if (strcmp(snapMode, "mini-center") == 0)
				def->snapMode = SNAP_MINI_CENTER;
			free(snapMode);
		}
	}

	err = cJSON_GetErrorPtr();
	if (err)
	{
		printf("json error\n%s\n", err);
		exit(1);
	}
	cJSON_Delete(json);
}
