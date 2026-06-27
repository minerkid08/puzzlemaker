#include "cjson.h"
#include "dynList.h"
#include "item/item.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "jsonUtils.h"
#include "utils.h"
#include "voxel/voxel.h"
#include <stdio.h>
#include <string.h>

extern Item* itemList;

char filename[256];
void save(const char* name)
{
	snprintf(filename, 256, "%s.chamb", name);
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
			if (kv->def->type == TYPE_INT)
				item = cJSON_CreateNumber(kv->value.i);
			if (kv->def->type == TYPE_BOOL)
				item = cJSON_CreateBool(kv->value.b);
			if (kv->def->type & TYPE_DROPDOWN)
				item = cJSON_CreateNumber(kv->value.i);
			cJSON_AddItemToObject(kvList, kv->def->name, item);
		}
	}

	char* str = cJSON_PrintUnformatted(json);

	FILE* file = fopen(filename, "wb");
	fwrite(str, strlen(str), 1, file);
	fclose(file);

	free(str);
}

void load(const char* name)
{
	snprintf(filename, 256, "%s.chamb", name);
	printf("loading '%s'\n", filename);

	for (int i = 0; i < dynList_size(itemList); i++)
	{
		removeItem(&itemList[i]);
	}

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

		v->portalability[0] = jsonGetBool(voxel, "portal0");
		v->portalability[1] = jsonGetBool(voxel, "portal1");
		v->portalability[2] = jsonGetBool(voxel, "portal2");
		v->portalability[3] = jsonGetBool(voxel, "portal3");
		v->portalability[4] = jsonGetBool(voxel, "portal4");
		v->portalability[5] = jsonGetBool(voxel, "portal5");
	}

	int itemCount = jsonGetInt(json, "itemCount");

	dynList_resize((void**)&itemList, itemCount);
	for (int i = 0; i < itemCount; i++)
		itemList[i].index = -1;

	cJSON* items = cJSON_GetObjectItem(json, "items");
	cJSON* itemJson;
	cJSON_ArrayForEach(itemJson, items)
	{
		int index = jsonGetInt(itemJson, "index");

		Item* item = 0;
		const char* id = cJSON_GetObjectItem(itemJson, "id")->valuestring;
		ItemDefinition* defs = getItemDefinitions();
    printf("loading item %s\n", id);

		for (int i = 0; i < dynList_size(defs); i++)
		{
			if (strcmp(defs[i].name, id) == 0)
				item = addItem(i, 0);
		}
    if(item == 0)
      errorf("failed to load item, item definition not found %s\n", id);

    jsonGetVec3(itemJson, "pos", item->pos);
    jsonGetVec3(itemJson, "rot", item->dir);

		if (item->def->type == ITEM_TYPE_PANEL)
			panelItemLoad(item, itemJson);

		if (item->def->type == ITEM_TYPE_VOLUME)
			volumeItemLoad(item, itemJson);

		updateItemTransform(item);

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

			const char* outputName = cJSON_GetObjectItem(outputJson, "output")->valuestring;
			OutputDef* outputs = item->def->outputs;
			for (int j = 0; j < dynList_size(outputs); j++)
			{
				if (strcmp(outputName, outputs[j].name) == 0)
					output->def = &outputs[j];
			}
      if(output->def == 0)
        errorf("failed to load item def %s, %d\n",item->def->name, i);

			const char* inputName = jsonGetStr(outputJson, "input");
			output->input = (InputDef*)inputName;
		}

		cJSON* kvJson = cJSON_GetObjectItem(itemJson, "kv");
		int len = dynList_size(item->def->kvs);
		for (int i = 0; i < len; i++)
		{
			ItemKvDef* def = &item->def->kvs[i];
			ItemKv* kv = &item->kv[i];

			if (cJSON_GetObjectItem(kvJson, def->name))
			{
				if (def->type == TYPE_INT)
					kv->value.i = jsonGetInt(kvJson, def->name);
				if (def->type == TYPE_BOOL)
					kv->value.b = jsonGetBool(kvJson, def->name);
				if (def->type & TYPE_DROPDOWN)
					kv->value.i = jsonGetInt(kvJson, def->name);
			}
			else
				kv->value = def->defaultValue;
		}
	}
	for (int i = 0; i < itemCount; i++)
	{
		Item* item = getItem(i);
		if (item->index == -1)
			continue;
    printf("loading outputs for %s\n", item->def->name);
    int len = dynList_size(item->outputs);
		for (int j = 0; j < len; j++)
		{
			ItemOutput* output = &item->outputs[j];
			char* inputName = (char*)output->input;

			Item* item2 = getItem(output->entity);
			InputDef* inputs = item2->def->inputs;
			for (int k = 0; k < dynList_size(inputs); k++)
			{
				if (strcmp(inputName, inputs[k].name) == 0)
					output->input = &inputs[k];
			}
			free(inputName);
		}
	}
  printf("done\n");
	cJSON_free(json);
}
