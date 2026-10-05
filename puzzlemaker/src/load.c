#include "antline/antline.h"
#include "cjson.h"
#include "dynList.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "mapsettings.h"
#include "utils.h"
#include "voxel/voxel.h"
#include <stdio.h>
#include <string.h>

extern Item* itemList;
extern Antline* antlines;

void load()
{
	char filename[256];
	snprintf(filename, 256, "maps/%s.chamb", mapSettings.name);
	printf("loading '%s'\n", filename);

	for (int i = 0; i < dynList_size(itemList); i++)
	{
		Item* item = &itemList[i];
		if (isItemValid(item))
			removeItem(item);
	}

	for (int i = 0; i < dynList_size(antlines); i++)
	{
		Antline* antline = &antlines[i];
		free(antline->segments);
	}
	dynList_resize((void**)&antlines, 0);

	FILE* file = fopen(filename, "rb");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);
	fclose(file);

	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				getVoxel(x, y, z)->solid = 1;
			}
		}
	}

	cJSON* voxelArr = cJSON_GetObjectItem(json, "voxels");
	cJSON* voxel;
	cJSON_ArrayForEach(voxel, voxelArr)
	{
		int x = jsonGetInt(voxel, "x");
		int y = jsonGetInt(voxel, "y");
		int z = jsonGetInt(voxel, "z");
		Voxel* v = getVoxel(x, y, z);

		v->solid = jsonGetBool(voxel, "solid");

		cJSON* portal = cJSON_GetObjectItem(voxel, "portal0");
		if (cJSON_IsBool(portal))
		{
			v->portalability[0] = jsonGetBool(voxel, "portal0");
			v->portalability[1] = jsonGetBool(voxel, "portal1");
			v->portalability[2] = jsonGetBool(voxel, "portal2");
			v->portalability[3] = jsonGetBool(voxel, "portal3");
			v->portalability[4] = jsonGetBool(voxel, "portal4");
			v->portalability[5] = jsonGetBool(voxel, "portal5");
		}
		else
		{
			v->portalability[0] = jsonGetInt(voxel, "portal0");
			v->portalability[1] = jsonGetInt(voxel, "portal1");
			v->portalability[2] = jsonGetInt(voxel, "portal2");
			v->portalability[3] = jsonGetInt(voxel, "portal3");
			v->portalability[4] = jsonGetInt(voxel, "portal4");
			v->portalability[5] = jsonGetInt(voxel, "portal5");
		}
	}

	cJSON* items = cJSON_GetObjectItem(json, "items");
	int itemCount = cJSON_GetArraySize(items);

	dynList_resize((void**)&itemList, itemCount);
	for (int i = 0; i < itemCount; i++)
	{
		itemList[i].index = -1;
		itemList[i].def = 0;
	}

	cJSON* itemJson;
	cJSON_ArrayForEach(itemJson, items)
	{
		int index = jsonGetInt(itemJson, "index");

		Item* item = 0;
		const char* id = cJSON_GetObjectItem(itemJson, "id")->valuestring;
		ItemDefinition* defs = getItemDefinitions();

		for (int i = 0; i < dynList_size(defs); i++)
		{
			if (strcmp(defs[i].name, id) == 0)
				item = addItem(i, 0);
		}
		if (item == 0)
			errorf("failed to load item, item definition not found for item '%s'\n", id);
		item->loadIndex = index;

		jsonGetVec3(itemJson, "pos", item->pos);
		jsonGetVec3(itemJson, "rot", item->dir);
		cJSON* snapDir = cJSON_GetObjectItem(json, "snapDir");
		if (snapDir)
			item->snapDir = cJSON_GetNumberValue(snapDir);

		item->def->callbacks->load(item, itemJson);

		updateItemTransformRot(item);

		cJSON* outputList = cJSON_GetObjectItem(itemJson, "outputs");
		int outputCount = cJSON_GetArraySize(outputList);
		dynList_resize((void**)&item->outputs, outputCount);
		for (int i = 0; i < outputCount; i++)
		{
			ItemOutput* output = &item->outputs[i];
			cJSON* outputJson = cJSON_GetArrayItem(outputList, i);

			int itemIndex = jsonGetInt(outputJson, "ent");
			output->entity = itemIndex;
			output->inverted = jsonGetBool(outputJson, "inverted");
			output->def = 0;
			if (cJSON_GetObjectItem(outputJson, "antline"))
				output->antline = jsonGetInt(outputJson, "antline");
			else
				output->antline = -1;

			const char* outputName = cJSON_GetObjectItem(outputJson, "output")->valuestring;
			OutputDef* outputs = item->def->outputs;
			for (int j = 0; j < dynList_size(outputs); j++)
			{
				if (strcmp(outputName, outputs[j].name) == 0)
					output->def = &outputs[j];
			}
			if (output->def == 0)
				errorf("failed to load item def %s, %d\n", item->def->name, i);

			const char* inputName = jsonGetStr(outputJson, "input");
			output->input = (InputDef*)inputName;
		}
		cJSON* kvJson = cJSON_GetObjectItem(itemJson, "kv");
		int len = dynList_size(item->def->kvs);
		for (int i = 0; i < len; i++)
		{
			ItemKv* kv = &item->kv[i];
			ItemKvDef* def = kv->def;

			int type = def->type;
			type &= ~(TYPE_INSTANCE);
			if (cJSON_GetObjectItem(kvJson, def->name))
			{
				if (type == TYPE_INT)
					kv->value.i = jsonGetInt(kvJson, def->name);
				if (type == TYPE_FLOAT)
					kv->value.f = jsonGetFloat(kvJson, def->name);
				if (type == TYPE_BOOL)
					kv->value.b = jsonGetBool(kvJson, def->name);
				if (type == TYPE_PICKER)
					kv->value.i = jsonGetInt(kvJson, def->name);
				if (type == TYPE_STRING)
				{
					char* str = cJSON_GetObjectItem(kvJson, def->name)->valuestring;
					strncpy(kv->value.s, str, 256);
				}
				if (type & TYPE_DROPDOWN)
				{
					int v = jsonGetInt(kvJson, def->name);
					int l = dynList_size(def->dropNames);

					if (v < 0 || v >= l)
						v = 0;
					kv->value.i = v;
				}
			}
		}
	}
	for (int i = 0; i < itemCount; i++)
	{
		Item* item = getItem(i);
		if (item->index == -1)
			continue;
		int len = dynList_size(item->outputs);
		for (int j = 0; j < len; j++)
		{
			ItemOutput* output = &item->outputs[j];
			char* inputName = (char*)output->input;

			Item* item2 = 0;
			for (int k = 0; k < itemCount; k++)
			{
				Item* item3 = getItem(k);
				if (item3->loadIndex == output->entity)
				{
					item2 = item3;
					output->entity = item2->index;
					break;
				}
			}
			InputDef* inputs = item2->def->inputs;
			for (int k = 0; k < dynList_size(inputs); k++)
			{
				if (strcmp(inputName, inputs[k].name) == 0)
					output->input = &inputs[k];
			}
			free(inputName);
		}

		len = dynList_size(item->def->kvs);
		for (int j = 0; j < len; j++)
		{
			ItemKv* kv = &item->kv[j];
			ItemKvDef* def = kv->def;

			int type = def->type;
			type &= ~(TYPE_INSTANCE);
			if (type != TYPE_PICKER)
				continue;
			Item* item2 = 0;
			for (int k = 0; k < itemCount; k++)
			{
				Item* item3 = getItem(k);
				if (item3->loadIndex == kv->value.i)
				{
					item2 = item3;
					kv->value.i = item2->index;
					break;
				}
			}
		}
	}
	cJSON* antlines = cJSON_GetObjectItem(json, "antlines");
	if (antlines)
	{
		cJSON* antlineJson;
		cJSON_ArrayForEach(antlineJson, antlines)
		{
			Antline* antline = addAntline();
			jsonGetVec3(antlineJson, "pos", antline->baseSegment.pos);
			jsonGetVec3(antlineJson, "rot", antline->baseSegment.rot);
			antline->hasCheck = jsonGetBool(antlineJson, "hasCheck");
			antlineUpdateTransformRot(&antline->baseSegment);
		}
	}

	cJSON* settings = cJSON_GetObjectItem(json, "settings");
	if (settings)
	{
		mapSettings.regen = jsonGetInt(settings, "regen");
		mapSettings.boots = jsonGetInt(settings, "boots");
		mapSettings.portalGun = jsonGetInt(settings, "portalgun");
		mapSettings.paintGun = jsonGetInt(settings, "paintgun");
		mapSettings.gameType = jsonGetInt(settings, "gametype");
		mapSettings.maxHealth = jsonGetInt(settings, "maxhealth");
		if (cJSON_GetObjectItem(settings, "maxExposure"))
		{
			PostProcessData* data = &mapSettings.postProcess;
			data->maxExposure = jsonGetFloat(settings, "maxExposure");
			data->minExposure = jsonGetFloat(settings, "minExposure");
			data->tonemapRate = jsonGetFloat(settings, "tonemapRate");
			data->brightPixelPercent = jsonGetFloat(settings, "brightPixelPercent");

			jsonGetVec3(settings, "primaryFogColor", data->primaryFogColor);
			jsonGetVec3(settings, "secondaryFogColor", data->secondaryFogColor);
			data->fogStart = jsonGetFloat(settings, "fogStart");
			data->fogEnd = jsonGetFloat(settings, "fogEnd");
			data->fogDensity = jsonGetFloat(settings, "fogDensity");
		}
	}

	printf("done\n");
	cJSON_Delete(json);
}
