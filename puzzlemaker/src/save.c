#include "antline/antline.h"
#include "camera.h"
#include "cjson.h"
#include "dynList.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "mapsettings.h"
#include "packages/theme.h"
#include "voxel/voxel.h"
#include <stdio.h>
#include <string.h>

extern Antline* antlines;

void save()
{
	char filename[256];
	snprintf(filename, 256, "maps/%s.chamb", mapSettings.name);
	printf("saving '%s'\n", filename);
	cJSON* json = cJSON_CreateObject();

	cJSON* voxelArr = cJSON_CreateArray();
	cJSON_AddItemToObject(json, "voxels", voxelArr);
	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				char shouldSave = 0;
				Voxel* v = getVoxel(x, y, z);
				if (v->solid == 0 || (v->solid && v->faces))
					shouldSave = 1;

				if (shouldSave == 0)
					continue;

				cJSON* voxel = cJSON_CreateObject();
				cJSON_AddItemToArray(voxelArr, voxel);
				cJSON_AddNumberToObject(voxel, "x", x);
				cJSON_AddNumberToObject(voxel, "y", y);
				cJSON_AddNumberToObject(voxel, "z", z);
				cJSON_AddBoolToObject(voxel, "solid", v->solid);

				if (v->solid)
				{
					cJSON_AddNumberToObject(voxel, "portal0", v->portalability[0]);
					cJSON_AddNumberToObject(voxel, "portal1", v->portalability[1]);
					cJSON_AddNumberToObject(voxel, "portal2", v->portalability[2]);
					cJSON_AddNumberToObject(voxel, "portal3", v->portalability[3]);
					cJSON_AddNumberToObject(voxel, "portal4", v->portalability[4]);
					cJSON_AddNumberToObject(voxel, "portal5", v->portalability[5]);
				}
			}
		}
	}

	cJSON* items = cJSON_CreateArray();
	cJSON_AddItemToObject(json, "items", items);

	Item* itemList = getItemList();
	int len = dynList_size(itemList);
	for (int i = 0; i < len; i++)
	{
		Item* item = &itemList[i];
		if (!isItemValid(item))
			continue;
		cJSON* itemJson = cJSON_CreateObject();
		cJSON_AddItemToArray(items, itemJson);
		cJSON_AddNumberToObject(itemJson, "index", item->index);
		cJSON_AddStringToObject(itemJson, "id", item->def->id);
		cJSON_AddStringToObject(itemJson, "package", item->def->package->name);
		cJSON_AddBoolToObject(itemJson, "hidden", item->hidden);

		cJSON* pos = cJSON_CreateArray();
		cJSON_AddItemToObject(itemJson, "pos", pos);
		jsonArrSetFloat(pos, item->pos[0]);
		jsonArrSetFloat(pos, item->pos[1]);
		jsonArrSetFloat(pos, item->pos[2]);

		cJSON* rot = cJSON_CreateArray();
		cJSON_AddItemToObject(itemJson, "rot", rot);
		jsonArrSetFloat(rot, item->dir[0]);
		jsonArrSetFloat(rot, item->dir[1]);
		jsonArrSetFloat(rot, item->dir[2]);

		cJSON_AddNumberToObject(itemJson, "snapDir", item->snapDir);

		item->def->callbacks->save(item, itemJson);

		cJSON* output = cJSON_CreateArray();
		cJSON_AddItemToObject(itemJson, "outputs", output);
		int outputLen = dynList_size(item->outputs);
		for (int i = 0; i < outputLen; i++)
		{
			ItemOutput* outputItem = &item->outputs[i];
			if (outputItem->input == 0)
				continue;
			cJSON* outputJson = cJSON_CreateObject();
			cJSON_AddItemToArray(output, outputJson);

			cJSON_AddNumberToObject(outputJson, "ent", outputItem->entity);
			cJSON_AddNumberToObject(outputJson, "antline", outputItem->antline);
			cJSON_AddBoolToObject(outputJson, "inverted", outputItem->inverted);
			cJSON_AddBoolToObject(outputJson, "onlyPrimary", outputItem->onlyPrimary);
			cJSON_AddStringToObject(outputJson, "input", outputItem->input->name);
			cJSON_AddStringToObject(outputJson, "output", outputItem->def->name);
		}

		cJSON* kvList = cJSON_CreateObject();
		cJSON_AddItemToObject(itemJson, "kv", kvList);
		int l = dynList_size(item->kv);
		for (int i = 0; i < l; i++)
		{
			ItemKv* kv = &item->kv[i];
			cJSON* item;
			int type = kv->def->type;
			type &= ~(TYPE_INSTANCE);
			if (type == TYPE_INT)
				item = cJSON_CreateNumber(kv->value.i);
			if (type == TYPE_FLOAT)
				item = cJSON_CreateNumber(kv->value.f);
			if (type == TYPE_BOOL)
				item = cJSON_CreateBool(kv->value.b);
			if (type == TYPE_STRING)
				item = cJSON_CreateString(kv->value.s);
			if (type == TYPE_BITFIELD)
			{
				int value = 0;
				int len = dynList_size(kv->def->dropNames);
				for (int j = 0; j < len; j++)
				{
					if (kv->value.s[j])
						value |= 1 << kv->def->dropValues[j].i;
				}
				item = cJSON_CreateNumber(value);
			}
			if (type == TYPE_PICKER)
				item = cJSON_CreateNumber(kv->value.i);
			if (type & TYPE_DROPDOWN)
				item = cJSON_CreateNumber(kv->value.i);
			cJSON_AddItemToObject(kvList, kv->def->name, item);
		}
	}
	cJSON* antlineList = cJSON_CreateArray();

	cJSON_AddItemToObject(json, "antlines", antlineList);

	len = dynList_size(antlines);
	for (int i = 0; i < len; i++)
	{
		Antline* antline = &antlines[i];
		if (!isAntlineValid(antline))
			continue;
		cJSON* antlineJson = cJSON_CreateObject();
		cJSON_AddItemToArray(antlineList, antlineJson);

		cJSON_AddNumberToObject(antlineJson, "id", antline->id);
		cJSON* pos = cJSON_CreateArray();
		cJSON_AddItemToObject(antlineJson, "pos", pos);
		jsonArrSetFloat(pos, antline->baseSegment.pos[0]);
		jsonArrSetFloat(pos, antline->baseSegment.pos[1]);
		jsonArrSetFloat(pos, antline->baseSegment.pos[2]);

		cJSON* rot = cJSON_CreateArray();
		cJSON_AddItemToObject(antlineJson, "rot", rot);
		jsonArrSetFloat(rot, antline->baseSegment.rot[0]);
		jsonArrSetFloat(rot, antline->baseSegment.rot[1]);
		jsonArrSetFloat(rot, antline->baseSegment.rot[2]);

		cJSON_AddBoolToObject(antlineJson, "hasCheck", antline->hasCheck);

		cJSON* segments = cJSON_CreateArray();
		cJSON_AddItemToObject(antlineJson, "segments", segments);

		int segCount = dynList_size(antline->segments);
		for (int j = 0; j < segCount; j++)
		{
			cJSON* segJson = cJSON_CreateObject();
			cJSON_AddItemToArray(segments, segJson);
			AntlineSegment* antlineSeg = &antline->segments[j];

			cJSON* pos = cJSON_CreateArray();
			cJSON_AddItemToObject(segJson, "pos", pos);
			jsonArrSetFloat(pos, antlineSeg->pos[0]);
			jsonArrSetFloat(pos, antlineSeg->pos[1]);
			jsonArrSetFloat(pos, antlineSeg->pos[2]);

			cJSON* rot = cJSON_CreateArray();
			cJSON_AddItemToObject(segJson, "rot", rot);
			jsonArrSetFloat(rot, antlineSeg->rot[0]);
			jsonArrSetFloat(rot, antlineSeg->rot[1]);
			jsonArrSetFloat(rot, antlineSeg->rot[2]);
			cJSON_AddNumberToObject(segJson, "length", antlineSeg->len);
		}
	}

	cJSON* settings = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "settings", settings);
	cJSON_AddStringToObject(settings, "theme", currentTheme->name);
	cJSON_AddNumberToObject(settings, "regen", mapSettings.regen);
	cJSON_AddNumberToObject(settings, "boots", mapSettings.boots);
	cJSON_AddNumberToObject(settings, "portalgun", mapSettings.portalGun);
	cJSON_AddNumberToObject(settings, "paintgun", mapSettings.paintGun);
	cJSON_AddNumberToObject(settings, "gametype", mapSettings.gameType);
	cJSON_AddNumberToObject(settings, "maxhealth", mapSettings.maxHealth);

	PostProcessData* data = &mapSettings.postProcess;

	cJSON_AddNumberToObject(settings, "maxExposure", data->maxExposure);
	cJSON_AddNumberToObject(settings, "minExposure", data->minExposure);
	cJSON_AddNumberToObject(settings, "tonemapRate", data->tonemapRate);
	cJSON_AddNumberToObject(settings, "brightPixelPercent", data->brightPixelPercent);
	jsonSetVec3(settings, "primaryFogColor", data->primaryFogColor);
	jsonSetVec3(settings, "secondaryFogColor", data->secondaryFogColor);
	cJSON_AddNumberToObject(settings, "fogStart", data->fogStart);
	cJSON_AddNumberToObject(settings, "fogEnd", data->fogEnd);
	cJSON_AddNumberToObject(settings, "fogDensity", data->fogDensity);

	cJSON_AddNumberToObject(settings, "minFalloff", data->minFalloff);
	cJSON_AddNumberToObject(settings, "maxFalloff", data->maxFalloff);
	cJSON_AddNumberToObject(settings, "maxWeight", data->maxWeight);
	cJSON_AddNumberToObject(settings, "fadeInDuration", data->fadeInDuration);
	cJSON_AddNumberToObject(settings, "fadeOutDuration", data->fadeOutDuration);
	cJSON_AddStringToObject(settings, "filename", data->filename);

	cJSON* editor = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "editor", editor);
	jsonSetVec3(editor, "cameraPos", cameraPos);
	jsonSetVec3(editor, "cameraRot", cameraRot);

	char* str = cJSON_Print(json);

	FILE* file = fopen(filename, "wb");
	fwrite(str, strlen(str), 1, file);
	fclose(file);

	free(str);
	cJSON_Delete(json);
	printf("saved\n");
}
