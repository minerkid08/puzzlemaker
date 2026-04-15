#include "cjson.h"
#include "dynList.h"
#include "item.h"
#include "jsonUtils.h"
#include "voxel.h"
#include <stdio.h>
#include <string.h>

char filename[64];
void save(const char* name)
{
	snprintf(filename, 64, "%s.chamb", name);
	cJSON* json = cJSON_CreateObject();

	cJSON* zArr = cJSON_CreateArray();
	cJSON_AddItemToObject(json, "voxels", zArr);
	for (int z = 0; z < MAP_SIZE; z++)
	{
		cJSON* yArr = cJSON_CreateArray();
		cJSON_AddItemToArray(zArr, yArr);
		for (int y = 0; y < MAP_SIZE; y++)
		{
			cJSON* xArr = cJSON_CreateArray();
			cJSON_AddItemToArray(yArr, xArr);
			for (int x = 0; x < MAP_SIZE; x++)
			{
				cJSON* voxel = cJSON_CreateObject();
				cJSON_AddItemToArray(xArr, voxel);
				Voxel* v = getVoxel(x, y, z);
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

	Item* itemList = *getItems();
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
		cJSON_AddNumberToObject(itemJson, "id", item->id);

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
    for(int i = 0; i < l; i++)
    {
      ItemKv* kv = &item->kv[i];
      cJSON* item;
      if(kv->def->type == TYPE_INT)
        item = cJSON_CreateNumber(kv->value.i);
      if(kv->def->type == TYPE_BOOL)
        item = cJSON_CreateBool(kv->value.b);
      if(kv->def->type & TYPE_DROPDOWN)
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
	snprintf(filename, 64, "%s.chamb", name);
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

	cJSON* voxelz = cJSON_GetObjectItem(json, "voxels");
	cJSON* voxely;
	int z = 0;
	cJSON_ArrayForEach(voxely, voxelz)
	{
		int y = 0;
		cJSON* voxelx;
		cJSON_ArrayForEach(voxelx, voxely)
		{
			int x = 0;
			cJSON* voxel;
			cJSON_ArrayForEach(voxel, voxelx)
			{
				Voxel* v = getVoxel(x, y, z);

				v->solid = jsonGetBool(voxel, "solid");

				v->portalability[0] = jsonGetBool(voxel, "portal0");
				v->portalability[1] = jsonGetBool(voxel, "portal1");
				v->portalability[2] = jsonGetBool(voxel, "portal2");
				v->portalability[3] = jsonGetBool(voxel, "portal3");
				v->portalability[4] = jsonGetBool(voxel, "portal4");
				v->portalability[5] = jsonGetBool(voxel, "portal5");
				x++;
			}
			y++;
		}
		z++;
	}

	int itemCount = jsonGetInt(json, "itemCount");
	Item** itemList = getItems();

	dynList_resize((void**)itemList, itemCount);

	cJSON* items = cJSON_GetObjectItem(json, "items");
	cJSON* itemJson;
	cJSON_ArrayForEach(itemJson, items)
	{
		int index = jsonGetInt(itemJson, "index");
		Item* item = &(*itemList)[index];

		item->index = index;
		item->id = jsonGetInt(itemJson, "id");

		cJSON* pos = cJSON_GetObjectItem(itemJson, "pos");
		item->pos[0] = jsonArrGetFloat(pos, 0);
		item->pos[1] = jsonArrGetFloat(pos, 1);
		item->pos[2] = jsonArrGetFloat(pos, 2);

		cJSON* rot = cJSON_GetObjectItem(itemJson, "rot");
		item->dir[0] = jsonArrGetFloat(rot, 0);
		item->dir[1] = jsonArrGetFloat(rot, 1);
		item->dir[2] = jsonArrGetFloat(rot, 2);

    item->def = &getItemDefinitions()[item->id];
    updateItemTransform(item);

    cJSON* outputList = cJSON_GetObjectItem(itemJson, "outputs");
    int outputCount = cJSON_GetArraySize(outputList);
		item->outputs = dynList_new(outputCount, sizeof(OutputDef));
    for(int i = 0; i < outputCount; i++)
    {
      ItemOutput* output = &item->outputs[i];
      cJSON* outputJson = cJSON_GetArrayItem(outputList, i);

      int itemIndex = jsonGetInt(outputJson, "ent");
      output->entity = itemIndex;
      output->inverted = jsonGetBool(outputJson, "inverted");

      const char* inputName = jsonGetStr(outputJson, "output");
      OutputDef* outputs = item->def->outputs;
      for(int j = 0; j < dynList_size(outputs); j++)
      {
        if(strcmp(inputName, outputs[j].name) == 0)
          output->def = &outputs[j];
      }
      free((void*)inputName);

      const char* outputName = jsonGetStr(outputJson, "input");
      Item* item = getItem(output->entity);
      InputDef* inputs = item->def->inputs;
      for(int j = 0; j < dynList_size(inputs); j++)
      {
        if(strcmp(inputName, inputs[j].name) == 0)
          output->input = &inputs[j];
      }
      free((void*)inputName);
    }

    cJSON* kvJson = cJSON_GetObjectItem(itemJson, "kv");
    int len = dynList_size(item->def->kvs);
		item->kv = dynList_new(len, sizeof(ItemKv));
    for(int i = 0; i < len; i++)
    {
      ItemKvDef* def = &item->def->kvs[i];
      ItemKv* kv = &item->kv[i];
      kv->def = def;

      if(def->type == TYPE_INT)
        kv->value.i = jsonGetInt(kvJson, def->name);
      if(def->type == TYPE_BOOL)
        kv->value.b = jsonGetBool(kvJson, def->name);
      if(def->type & TYPE_DROPDOWN)
        kv->value.i = jsonGetInt(kvJson, def->name);
    }
	}
  cJSON_free(json);
}
