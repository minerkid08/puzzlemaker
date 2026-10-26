#include "entityItem.h"
#include "assetManager.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "cjson.h"
#include "dynList.h"
#include "export/entity.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "packages/package.h"
#include "renderer/mesh.h"
#include "utils.h"
#include <math.h>
#include <string.h>

static ItemCallbacks callbacks;

static __attribute__((constructor)) void init()
{
	callbacks.init = entityItemInit;
	callbacks.exportItem = entityItemExport;
	callbacks.render = entityItemRender;
	callbacks.getBoundingBox = entityItemGetBoundingBox;
	callbacks.save = entityItemSave;
	callbacks.load = entityItemLoad;
}

void* loadEntityItemDef(cJSON* item, ItemDefinition* itemDef)
{
	itemDef->callbacks = &callbacks;
	EntityItemDef* def = malloc(sizeof(EntityItemDef));
	const char* modelName = jsonGetStr(item, "model");

	vec3 bound1;
	vec3 bound2;

	jsonGetVec3(item, "bound1", bound1);
	jsonGetVec3(item, "bound2", bound2);

	def->bound1[0] = fminf(bound1[0], bound2[0]);
	def->bound1[1] = fminf(bound1[1], bound2[1]);
	def->bound1[2] = fminf(bound1[2], bound2[2]);
	def->bound2[0] = fmaxf(bound1[0], bound2[0]);
	def->bound2[1] = fmaxf(bound1[1], bound2[1]);
	def->bound2[2] = fmaxf(bound1[2], bound2[2]);

  jsonGetTransform(item, "transform", def->positionOffset, def->rotationOffset, 0);
  jsonGetTransform(item, "editorTransform", def->editorPos, def->editorRot, def->editorTransform);

	if (cJSON_HasObjectItem(item, "instance"))
	{
		def->entityName = 0;
		def->instanceName = jsonGetStrD(item, "instance");
	}
	else
	{
		def->instanceName = 0;
		def->entityName = jsonGetStrD(item, "entity");
	}

	def->mesh = assetManagerLoadMesh(modelName);
	itemDef->flags = def->mesh->flags;
	return def;
}

void entityItemInit(Item* item)
{
	item->data = 0;
}

void entityItemRender(Item* item)
{
	EntityItemDef* def = item->def->data;
	mat4 transform;
	mat4 editorTransform;
	memcpy(editorTransform, def->editorTransform, sizeof(mat4));
	memcpy(transform, item->transform, sizeof(mat4));
	glm_mat4_mul(transform, editorTransform, transform);
	renderMesh(def->mesh, transform);
}

void entityItemExport(Item* item)
{
	EntityItemDef* defData = item->def->data;

	char buf[100];
	snprintf(buf, 100, "%s%d", item->def->id, item->index);

	vec3 itemPos;
	memcpy(itemPos, item->pos, sizeof(vec3));
	vec4 itemRot;
	memcpy(itemRot, item->quat, sizeof(vec4));

	mat4 transform;
	mat4 rotMat;
	glm_mat4_identity(transform);
	glm_translate(transform, itemPos);
	glm_quat_mat4(itemRot, rotMat);
	glm_mat4_mul(transform, rotMat, transform);

	Entity* entity = exportCreateEntity();

	vec3 entPos;
	entPos[2] = defData->positionOffset[0] / 64.0f;
	entPos[0] = defData->positionOffset[1] / 64.0f;
	entPos[1] = defData->positionOffset[2] / 64.0f;

	glm_mat4_mulv3(transform, entPos, 1, entPos);
	memcpy(entity->pos, entPos, sizeof(vec3));

	vec3 entRot;
	memcpy(entRot, defData->rotationOffset, sizeof(vec3));
	entRot[0] = glm_rad(entRot[0]);
	entRot[1] = glm_rad(entRot[1]);
	entRot[2] = glm_rad(entRot[2]);
	vec4 quat;
	glm_quat_identity(quat);
	glm_euler_yxz_quat(entRot, quat);

	vec4 newRot;
	glm_quat_mul(itemRot, quat, newRot);
	glm_quat_mat4(newRot, rotMat);

	getEulerAngles(rotMat, entRot);
	entRot[0] = glm_deg(entRot[0]);
	entRot[1] = glm_deg(entRot[1]);
	entRot[2] = glm_deg(entRot[2]);
	memcpy(entity->rotation, entRot, sizeof(vec3));

	entity->name = strdup(buf);
	if (defData->instanceName)
	{
		entity->className = "func_instance";
    char* instanceName = itemProcessString(defData->instanceName, item->kv);
		int offset = 0;
		Package* package = processPackageString(item->def->package, instanceName, &offset);
		char* n = strdupf("pmInstances/%s/%s", package->name, instanceName + offset);
		exportEntityAddKvss(entity, "file", n);
    free(instanceName);
    free(n);
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

void entityItemGetBoundingBox(Item* item, vec3 min, vec3 max)
{
	EntityItemDef* defData = item->def->data;

	memcpy(min, defData->bound1, sizeof(vec3));
	memcpy(max, defData->bound2, sizeof(vec3));
}

void entityItemSave(Item* item, cJSON* json)
{
}

void entityItemLoad(Item* item, cJSON* json)
{
}
