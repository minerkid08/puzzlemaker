#include "entityItem.h"
#include "assetManager.h"
#include "cjson.h"
#include "dynList.h"
#include "export/entity.h"
#include "jsonUtils.h"
#include "renderer/renderer.h"
#include <string.h>

void* loadEntityItemDef(cJSON* item)
{
	EntityItemDef* def = malloc(sizeof(EntityItemDef));
	const char* modelName = cJSON_GetObjectItem(item, "model")->valuestring;
	const char* textureName = cJSON_GetObjectItem(item, "mat")->valuestring;

	if (cJSON_HasObjectItem(item, "instance"))
	{
		def->entityName = 0;
		def->instanceName = jsonGetStr(item, "instance");
	}
	else
	{
		def->instanceName = 0;
		def->entityName = jsonGetStr(item, "entity");
	}

	def->mesh = assetManagerLoadMesh(modelName);
	def->texture = assetManagerLoadTexture(textureName);

	return def;
}

void entityItemInit(Item* item)
{
}

void entityItemRender(Item* item)
{
	EntityItemDef* def = item->def->data;
	drawMesh(def->mesh, def->texture, item->transform);
}

void entityItemExport(Item* item)
{
	EntityItemDef* defData = item->def->data;

	char buf[100];
	snprintf(buf, 100, "%s%d", item->def->name, item->index);

	float x = item->pos[0] + item->def->offset[0];
	float y = item->pos[1] + item->def->offset[1];
	float z = item->pos[2] + item->def->offset[2];

	Entity* entity = exportCreateEntity();
	entity->pos[0] = x;
	entity->pos[1] = y;
	entity->pos[2] = z;
	entity->rotation[0] = item->dir[0];
	entity->rotation[1] = item->dir[1];
	entity->rotation[2] = item->dir[2];
	entity->name = strdup(buf);
	if (defData->instanceName)
	{
		entity->className = "func_instance";
    snprintf(buf, sizeof(buf), "puzzlemakerInstances/%s", defData->instanceName);
		exportEntityAddKvss(entity, "file", buf);
	}
	else
		entity->className = defData->entityName;

	int l = dynList_size(item->def->staticKvs);
	for (int i = 0; i < l; i++)
		exportEntityAddKvs(entity, item->def->staticKvs[i]);

	l = dynList_size(item->def->kvs);
	for (int i = 0; i < l; i++)
	{
		ItemKv* kv = &item->kv[i];
		exportEntityAddKv(entity, kv);
	}

	if (dynList_size(item->outputs))
		entity->outputs = item->outputs;
}
