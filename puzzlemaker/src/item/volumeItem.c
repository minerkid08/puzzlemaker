#include "item/volumeItem.h"
#include "assetManager.h"
#include "cglm/types.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "jsonUtils.h"
#include "renderer/renderer.h"
#include <string.h>

void* loadVolumeItemDef(cJSON* item)
{
	VolumeItemDef* data = malloc(sizeof(VolumeItemDef));

	if (cJSON_GetObjectItem(item, "minSize"))
		jsonGetVec3(item, "minSize", data->minSize);
	else
	{
		data->minSize[0] = 0;
		data->minSize[1] = 0;
		data->minSize[2] = 0;
	}
	if (cJSON_GetObjectItem(item, "maxSize"))
		jsonGetVec3(item, "maxSize", data->maxSize);
	else
	{
		data->maxSize[0] = 99999;
		data->maxSize[1] = 99999;
		data->maxSize[2] = 99999;
	}
	if (cJSON_GetObjectItem(item, "defaultSize"))
		jsonGetVec3(item, "defaultSize", data->defaultSize);
	else
	{
		data->defaultSize[0] = 4;
		data->defaultSize[1] = 4;
		data->defaultSize[2] = 4;
	}

	const char* filename = cJSON_GetObjectItem(item, "editorTexture")->valuestring;
	data->material = assetManagerLoadTexture(filename);
	data->exportMaterial = jsonGetStr(item, "texture");
	if (cJSON_GetObjectItem(item, "entity"))
		data->entity = jsonGetStr(item, "entity");
	else
		data->entity = 0;

	return data;
}

void volumeItemInit(Item* item)
{
	VolumeItemDef* def = item->def->data;
	VolumeItemData* data = malloc(sizeof(VolumeItemData));
	data->size[0] = def->defaultSize[0];
	data->size[1] = def->defaultSize[1];
	data->size[2] = def->defaultSize[2];
	item->data = data;
}

void setVec(vec3 vec, float x, float y, float z)
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
}

void volumeItemRender(Item* item)
{
	VolumeItemDef* def = item->def->data;
	VolumeItemData* data = item->data;

	float x = data->size[0];
	float y = data->size[1];
	float z = data->size[2];

	mat4 transform;
	memcpy(transform, item->transform, sizeof(mat4));

	vec3 verts[8];
	setVec(verts[0], 0, 0, 0);
	setVec(verts[1], x, 0, 0);
	setVec(verts[2], 0, y, 0);
	setVec(verts[3], x, y, 0);
	setVec(verts[4], 0, 0, z);
	setVec(verts[5], x, 0, z);
	setVec(verts[6], 0, y, z);
	setVec(verts[7], x, y, z);

	drawRect(verts[1], verts[0], verts[3], verts[2], def->material);
	drawRect(verts[4], verts[5], verts[6], verts[7], def->material);
	drawRect(verts[0], verts[4], verts[2], verts[6], def->material);
	drawRect(verts[5], verts[1], verts[7], verts[3], def->material);
	drawRect(verts[0], verts[1], verts[4], verts[5], def->material);
	drawRect(verts[2], verts[6], verts[3], verts[7], def->material);
	panelEndFrame(transform, 1);
}

void volumeItemExport(Item* item)
{
	VolumeItemDef* def = item->def->data;
	VolumeItemData* data = item->data;

	vec3 start = {0, 0, 0};
	Brush* brush = exportCreateBrush(start, data->size);

	if (def->entity)
		brush->ent = 1;

	mat4 transform;
	memcpy(transform, item->transform, sizeof(mat4));

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];

		side->material = def->exportMaterial;
		for (int j = 0; j < 4; j++)
		{
			vec3 res;
			vec3 vert;
			memcpy(vert, side->verts[j], sizeof(vec3));
			glm_mat4_mulv3(transform, vert, 1, res);
			memcpy(side->verts[j], res, sizeof(vec3));
		}
	}

	if (def->entity)
	{
		Entity* ent = exportCreateEntity();
		char buf[20];
		snprintf(buf, 20, "%s%d", item->def->name, item->index);
		memcpy(ent->pos, item->pos, sizeof(vec3));
		memcpy(ent->rotation, item->dir, sizeof(vec3));
		ent->name = strdup(buf);
		ent->className = def->entity;
		exportEntityAddBrush(ent, brush);
		int l = dynList_size(item->def->staticKvs);
		for (int i = 0; i < l; i++)
			exportEntityAddKvs(ent, item->def->staticKvs[i]);

		l = dynList_size(item->def->kvs);
		for (int i = 0; i < l; i++)
		{
			ItemKv* kv = &item->kv[i];
			exportEntityAddKv(ent, kv);
		}
		if (dynList_size(item->outputs))
			ent->outputs = item->outputs;
	}
}

void volumeItemSave(Item* item, cJSON* json)
{
	VolumeItemData* data = item->data;
	cJSON* obj = cJSON_CreateObject();
	cJSON_AddNumberToObject(obj, "x", data->size[0]);
	cJSON_AddNumberToObject(obj, "y", data->size[1]);
	cJSON_AddNumberToObject(obj, "z", data->size[2]);
	cJSON_AddItemToObject(json, "size", obj);
}

void volumeItemLoad(Item* item, cJSON* json)
{
	VolumeItemData* data = item->data;
	cJSON* obj = cJSON_GetObjectItem(json, "size");
	data->size[0] = jsonGetFloat(obj, "x");
	data->size[1] = jsonGetFloat(obj, "y");
	data->size[2] = jsonGetFloat(obj, "z");
}
