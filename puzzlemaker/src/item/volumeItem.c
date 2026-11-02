#include "item/volumeItem.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cjson.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "item/item.h"
#include "renderer/renderer.h"
#include "utils.h"
#include <math.h>
#include <string.h>

static ItemCallbacks callbacks;

#ifdef __linux__
__attribute__((constructor)) static void volumeItemProcInit()
#else
void volumeItemProcInit()
#endif
{
	callbacks.init = volumeItemInit;
	callbacks.exportItem = volumeItemExport;
	callbacks.render = volumeItemRender;
	callbacks.getBoundingBox = volumeItemGetBoundingBox;
	callbacks.save = volumeItemSave;
	callbacks.load = volumeItemLoad;
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*volumeItemProcInitPtr)(void) = volumeItemProcInit;

#pragma comment(linker, "/include:volumeItemProcInitPtr")
#endif

void* loadVolumeItemDef(cJSON* item, ItemDefinition* itemDef)
{
	itemDef->callbacks = &callbacks;
	VolumeItemDef* data = malloc(sizeof(VolumeItemDef));

	itemDef->flags |= RESTRICT_X | RESTRICT_Y | RESTRICT_Z;

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

	char flags = 0;
	jsonGetEditorTex(item, "editorTexture", &data->material, &flags);
	itemDef->flags |= flags;

	if (cJSON_GetObjectItem(item, "texture"))
	{
		jsonGetTex(item, "texture", &data->exportMaterial[0]);
		for (int i = 0; i < 5; i++)
			memcpy(&data->exportMaterial[i + 1], &data->exportMaterial[0], sizeof(Texture));
	}
	else
	{
		cJSON* textureJson = cJSON_GetObjectItem(item, "textures");
		if (textureJson)
		{
			jsonGetTex(textureJson, "right", &data->exportMaterial[DIR_POS_X]);
			jsonGetTex(textureJson, "left", &data->exportMaterial[DIR_NEG_X]);
			jsonGetTex(textureJson, "top", &data->exportMaterial[DIR_POS_Y]);
			jsonGetTex(textureJson, "bottom", &data->exportMaterial[DIR_NEG_Y]);
			jsonGetTex(textureJson, "back", &data->exportMaterial[DIR_POS_Z]);
			jsonGetTex(textureJson, "front", &data->exportMaterial[DIR_NEG_Z]);
		}
		else
			errorf("no textures defined");
	}

	if (cJSON_GetObjectItem(item, "entity"))
		data->entity = jsonGetStrD(item, "entity");
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

static inline void setVec(vec3 vec, float x, float y, float z)
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

	vec3 verts[8];
	setVec(verts[0], 0, 0, 0);
	setVec(verts[1], x, 0, 0);
	setVec(verts[2], 0, y, 0);
	setVec(verts[3], x, y, 0);
	setVec(verts[4], 0, 0, z);
	setVec(verts[5], x, 0, z);
	setVec(verts[6], 0, y, z);
	setVec(verts[7], x, y, z);

	drawRect(verts[1], verts[0], verts[3], verts[2], &def->material);
	drawRect(verts[4], verts[5], verts[6], verts[7], &def->material);
	drawRect(verts[0], verts[4], verts[2], verts[6], &def->material);
	drawRect(verts[5], verts[1], verts[7], verts[3], &def->material);
	drawRect(verts[0], verts[1], verts[4], verts[5], &def->material);
	drawRect(verts[2], verts[6], verts[3], verts[7], &def->material);

	panelEndFrame(item->transform, 1, 1);
}

void volumeItemExport(Item* item)
{
	VolumeItemDef* def = item->def->data;
	VolumeItemData* data = item->data;

	vec3 start = {0, 0, 0};
	vec3 size2 = {data->size[0], data->size[1], data->size[2]};
	Brush* brush = exportCreateBrush(start, size2);

	if (def->entity)
		brush->ent = 1;

	brush->pos[0] = item->pos[0];
	brush->pos[1] = item->pos[1];
	brush->pos[2] = item->pos[2];
	brush->rot[0] = item->dir[0];
	brush->rot[1] = item->dir[1];
	brush->rot[2] = item->dir[2];

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];

		Texture* tex = &def->exportMaterial[i];
		side->material = tex->filename;
		side->texHeight = tex->size;
		side->texWidth = tex->size;
		side->alignment = tex->alignment;
		for (int j = 0; j < 4; j++)
		{
			vec3 res;
			vec3 vert;
			memcpy(vert, side->verts[j], sizeof(vec3));
			glm_mat4_mulv3(item->transform, vert, 1, res);
			memcpy(side->verts[j], res, sizeof(vec3));
		}
	}

	if (def->entity)
	{
		Entity* ent = exportCreateEntity();
		char buf[20];
		snprintf(buf, 20, "%s%d", item->def->id, item->index);
		memcpy(ent->pos, item->pos, sizeof(vec3));
		ent->rotation[0] = 0;
		ent->rotation[1] = 0;
		ent->rotation[2] = 0;
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

	VolumeItemDef* def = item->def->data;

	data->size[0] = fmaxf(data->size[0], def->minSize[0]);
	data->size[1] = fmaxf(data->size[1], def->minSize[1]);
	data->size[2] = fmaxf(data->size[2], def->minSize[2]);

	data->size[0] = fminf(data->size[0], def->maxSize[0]);
	data->size[1] = fminf(data->size[1], def->maxSize[1]);
	data->size[2] = fminf(data->size[2], def->maxSize[2]);
}

void volumeItemGetBoundingBox(Item* item, vec3 min, vec3 max)
{
	VolumeItemData* data = item->data;
	min[0] = 0;
	min[1] = 0;
	min[2] = 0;
	max[0] = data->size[0];
	max[1] = data->size[1];
	max[2] = data->size[2];
}
