#include "antline/antline.h"
#include "camera.h"
#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "mapsettings.h"
#include "packages/theme.h"
#include "utils.h"
#include "voxel/voxel.h"
#include <stdio.h>
#include <string.h>

extern Item* itemList;
extern Antline* antlines;
extern ItemGroup* groups;

static char filename[256];

void load()
{
	cJSON* json = 0;
	char* data = 0;
	FILE* file = 0;

	snprintf(filename, 256, "maps/%s.chamb", mapSettings.name);
	printf("loading '%s'\n", filename);
	int res;
	initErrorStackD(filename, res);
	if (res)
	{
		if(json)
			cJSON_Delete(json);
		if(data)
		{
			fclose(file);
			free(data);
		}
		for (int i = 0; i < dynList_size(itemList); i++)
		{
			Item* item = &itemList[i];
			if (isItemValid(item))
				removeItem(item);
		}
		initVoxels();
		openErrorPopup();
		return;
	}

	for (int i = 0; i < dynList_size(itemList); i++)
	{
		Item* item = &itemList[i];
		if (isItemValid(item))
			removeItem(item);
	}

	for (int i = 0; i < dynList_size(antlines); i++)
	{
		Antline* antline = &antlines[i];
		dynList_free(antline->segments);
	}
	dynList_resize((void**)&antlines, 0);

	file = fopen(filename, "rb");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	if(err)
		jsonParseError(data, err, filename);
	free(data);
	fclose(file);
	data = 0;
	file = 0;

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

	mapReigonStart[0] = 9999;
	mapReigonStart[1] = 9999;
	mapReigonStart[2] = 9999;
	mapReigonEnd[0] = -1;
	mapReigonEnd[1] = -1;
	mapReigonEnd[2] = -1;

	cJSON* voxelArr = jsonGetArray(json, "voxels");
	int voxelArrLen = cJSON_GetArraySize(voxelArr);
	for (int i = 0; i < voxelArrLen; i++)
	{
		cJSON* voxel = jsonArrGetObject(voxelArr, i);
		int x = jsonGetInt(voxel, "x");
		int y = jsonGetInt(voxel, "y");
		int z = jsonGetInt(voxel, "z");
		Voxel* v = getVoxel(x, y, z);

		v->solid = jsonGetBool(voxel, "solid");

		if (v->solid)
		{
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
		else
		{
			mapReigonStart[0] = min(mapReigonStart[0], x);
			mapReigonStart[1] = min(mapReigonStart[1], y);
			mapReigonStart[2] = min(mapReigonStart[2], z);
			mapReigonEnd[0] = max(mapReigonEnd[0], x);
			mapReigonEnd[1] = max(mapReigonEnd[1], y);
			mapReigonEnd[2] = max(mapReigonEnd[2], z);
		}
		popStackFrame();
	}
	popStackFrame();

	mapReigonStart[0]--;
	mapReigonStart[1]--;
	mapReigonStart[2]--;

	mapReigonEnd[0] += 2;
	mapReigonEnd[1] += 2;
	mapReigonEnd[2] += 2;

	cJSON* items = jsonGetArray(json, "items");
	int itemCount = cJSON_GetArraySize(items);

	dynList_resize((void**)&itemList, itemCount);
	for (int i = 0; i < itemCount; i++)
	{
		itemList[i].index = -1;
		itemList[i].def = 0;
	}

	updateVoxelFaces();

	for (int i = 0; i < itemCount; i++)
	{
		cJSON* itemJson = jsonArrGetObject(items, i);
		int index = jsonGetInt(itemJson, "index");

		Item* item = 0;
		const char* id = jsonGetStr(itemJson, "id");
		ItemDefinition* defs = getItemDefinitions();

		const char* groupName = jsonGetStr(itemJson, "group");
		for (int i = 0; i < dynList_size(groups); i++)
		{
			ItemGroup* group = &groups[i];
			if (strcmp(group->name, groupName))
				continue;
			for (int j = 0; j < group->size; j++)
			{
				if (strcmp(defs[j + group->startInd].id, id) == 0)
				{
					item = addItem(j + group->startInd, 0);
					break;
				}
			}
			if (item)
				break;
		}
		if (item == 0)
			errorf("failed to load item, item definition not found for item '%s'", id);
		item->loadIndex = index;

		jsonGetVec3(itemJson, "pos", item->pos);
		jsonGetVec3(itemJson, "rot", item->dir);
		item->hidden = jsonGetBoolC(itemJson, "hidden", 0);
		item->snapDir = jsonGetIntC(itemJson, "snapDir", 0);

		item->def->callbacks->load(item, itemJson);

		updateItemTransformRot(item);

		cJSON* outputList = jsonGetArray(itemJson, "outputs");
		int outputCount = cJSON_GetArraySize(outputList);
		dynList_resize((void**)&item->outputs, outputCount);
		for (int i = 0; i < outputCount; i++)
		{
			ItemOutput* output = &item->outputs[i];
			cJSON* outputJson = jsonArrGetObject(outputList, i);

			int itemIndex = jsonGetInt(outputJson, "ent");
			output->entity = itemIndex;
			output->inverted = jsonGetBool(outputJson, "inverted");
			output->onlyPrimary = jsonGetBoolC(outputJson, "onlyPrimary", 0);
			output->def = 0;
			output->antline = jsonGetIntC(outputJson, "antline", -1);

			const char* outputName = jsonGetStr(outputJson, "output");
			OutputDef* outputs = item->def->outputs;
			for (int j = 0; j < dynList_size(outputs); j++)
			{
				if (strcmp(outputName, outputs[j].name) == 0)
					output->def = &outputs[j];
			}
			if (output->def == 0)
				errorf("failed to load item def %s, %d", item->def->name, i);

			const char* inputName = jsonGetStrD(outputJson, "input");
			output->input = (InputDef*)inputName;
			popStackFrame();
		}
		popStackFrame();

		cJSON* kvJson = jsonGetObject(itemJson, "kv");
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
					const char* str = jsonGetStr(kvJson, def->name);
					strncpy(kv->value.s, str, 256);
				}
				if (type == TYPE_BITFIELD)
				{
					int value = jsonGetInt(kvJson, def->name);
					int len = dynList_size(kv->def->dropNames);
					for (int j = 0; j < len; j++)
					{
						if (value & (1 << kv->def->dropValues[j].i))
							kv->value.s[j] = 1;
						else
							kv->value.s[j] = 0;
					}
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
		popStackFrame();
	}
	popStackFrame();

	updateCulledVoxels();

	cJSON* antlineList = jsonGetArrayC(json, "antlines");
	if (antlineList)
	{
		int antlineCount = cJSON_GetArraySize(antlineList);
		for (int i = 0; i < antlineCount; i++)
		{
			cJSON* antlineJson = jsonArrGetObject(antlineList, i);
			Antline* antline = addAntline();
			antline->prevId = jsonGetInt(antlineJson, "id");
			jsonGetVec3(antlineJson, "pos", antline->baseSegment.pos);
			jsonGetVec3(antlineJson, "rot", antline->baseSegment.rot);
			antline->hasCheck = jsonGetBool(antlineJson, "hasCheck");
			antlineUpdateTransformRot(&antline->baseSegment);

			cJSON* segments = jsonGetArray(antlineJson, "segments");
			int l = cJSON_GetArraySize(segments);
			dynList_resize((void**)&antline->segments, l);
			for (int j = 0; j < l; j++)
			{
				cJSON* segmentJson = jsonArrGetObject(segments, j);
				AntlineSegment* seg = &antline->segments[j];
				jsonGetVec3(segmentJson, "pos", seg->pos);
				jsonGetVec3(segmentJson, "rot", seg->rot);
				seg->len = jsonGetInt(segmentJson, "length");
				antlineUpdateTransformRot(seg);
				popStackFrame();
			}
			popStackFrame();
			popStackFrame();
		}
		popStackFrame();
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

			if (output->antline != -1)
			{
				int antlineCount = dynList_size(antlines);
				for (int k = 0; k < antlineCount; k++)
				{
					Antline* antline = &antlines[k];
					if (output->antline == antline->prevId)
					{
						output->antline = antline->id;
						break;
					}
				}
			}
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

	cJSON* settings = jsonGetObjectC(json, "settings");
	if (settings)
	{
		const char* themeName = jsonGetStr(settings, "theme");
		Theme* theme = 0;
		for (int i = 0; i < dynList_size(themeList); i++)
		{
			Theme* t = &themeList[i];
			if (strcmp(themeName, t->name) == 0)
			{
				theme = t;
				break;
			}
		}
		if (theme)
			setTheme(theme);
		else
			errorf("unknown theme '%s'", themeName);
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
		popStackFrame();
	}

	cJSON* editor = jsonGetObjectC(json, "editor");
	if (editor)
	{
		jsonGetVec3(editor, "cameraPos", cameraPos);
		jsonGetVec3(editor, "cameraRot", cameraRot);
		popStackFrame();
	}

	printf("done\n");
	cJSON_Delete(json);
}

void resetMap()
{
	for (int i = 0; i < dynList_size(itemList); i++)
	{
		Item* item = &itemList[i];
		if (isItemValid(item))
			removeItem(item);
	}
	dynList_resize((void**)&itemList, 0);

	for (int i = 0; i < dynList_size(antlines); i++)
	{
		Antline* antline = &antlines[i];
		dynList_free(antline->segments);
	}
	dynList_resize((void**)&antlines, 0);

	initVoxels();
}
