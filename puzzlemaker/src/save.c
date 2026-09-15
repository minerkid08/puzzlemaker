#include "cjson.h"
#include "dynList.h"
#include "item/item.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "jsonUtils.h"
#include "mapsettings.h"
#include "voxel/voxel.h"
#include <stdio.h>
#include <string.h>

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
				if (v->solid)
				{
					if (z + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y, z + 1);
						if (!v2->solid)
							shouldSave = 1;
					}

					if (z - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y, z - 1);
						if (!v2->solid)
							shouldSave = 1;
					}

					if (x + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x + 1, y, z);
						if (!v2->solid)
							shouldSave = 1;
					}

					if (x - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x - 1, y, z);
						if (!v2->solid)
							shouldSave = 1;
					}

					if (y + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y + 1, z);
						if (!v2->solid)
							shouldSave = 1;
					}

					if (y - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y - 1, z);
						if (!v2->solid)
							shouldSave = 1;
					}
				}
				else
					shouldSave = 1;

				if (shouldSave == 0)
					continue;

				cJSON* voxel = cJSON_CreateObject();
				cJSON_AddItemToArray(voxelArr, voxel);
				cJSON_AddNumberToObject(voxel, "x", x);
				cJSON_AddNumberToObject(voxel, "y", y);
				cJSON_AddNumberToObject(voxel, "z", z);
				cJSON_AddBoolToObject(voxel, "solid", v->solid);

				cJSON_AddBoolToObject(voxel, "portal0", v->portalability[0]);
				cJSON_AddBoolToObject(voxel, "portal1", v->portalability[1]);
				cJSON_AddBoolToObject(voxel, "portal2", v->portalability[2]);
				cJSON_AddBoolToObject(voxel, "portal3", v->portalability[3]);
				cJSON_AddBoolToObject(voxel, "portal4", v->portalability[4]);
				cJSON_AddBoolToObject(voxel, "portal5", v->portalability[5]);
			}
		}
	}

	cJSON* items = cJSON_CreateArray();
	cJSON_AddItemToObject(json, "items", items);

	Item* itemList = getItemList();
	int len = dynList_size(itemList);
	cJSON_AddNumberToObject(json, "itemCount", len);
	for (int i = 0; i < len; i++)
	{
		Item* item = &itemList[i];
		if (item->index == -1)
			continue;
		cJSON* itemJson = cJSON_CreateObject();
		cJSON_AddItemToArray(items, itemJson);
		cJSON_AddNumberToObject(itemJson, "index", item->index);
		cJSON_AddStringToObject(itemJson, "id", item->def->name);

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

		if (item->def->type == ITEM_TYPE_PANEL)
			panelItemSave(item, itemJson);
		if (item->def->type == ITEM_TYPE_VOLUME)
			volumeItemSave(item, itemJson);

		cJSON* output = cJSON_CreateArray();
		cJSON_AddItemToObject(itemJson, "outputs", output);
		int outputLen = dynList_size(item->outputs);
		for (int i = 0; i < outputLen; i++)
		{
			ItemOutput* outputItem = &item->outputs[i];
      if(outputItem->input == 0)
        continue;
			cJSON* outputJson = cJSON_CreateObject();
			cJSON_AddItemToArray(output, outputJson);

			cJSON_AddNumberToObject(outputJson, "ent", outputItem->entity);
			cJSON_AddBoolToObject(outputJson, "inverted", outputItem->inverted);
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
			if (type & TYPE_DROPDOWN)
				item = cJSON_CreateNumber(kv->value.i);
			cJSON_AddItemToObject(kvList, kv->def->name, item);
		}
	}

	cJSON* settings = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "settings", settings);
	cJSON_AddNumberToObject(settings, "regen", mapSettings.regen);
	cJSON_AddNumberToObject(settings, "boots", mapSettings.boots);
	cJSON_AddNumberToObject(settings, "portalgun", mapSettings.portalGun);
	cJSON_AddNumberToObject(settings, "paintgun", mapSettings.paintGun);
	cJSON_AddNumberToObject(settings, "gametype", mapSettings.gameType);
	cJSON_AddNumberToObject(settings, "maxhealth", mapSettings.maxHealth);

	char* str = cJSON_Print(json);

	FILE* file = fopen(filename, "wb");
	fwrite(str, strlen(str), 1, file);
	fclose(file);

	free(str);
  cJSON_free(json);
}
