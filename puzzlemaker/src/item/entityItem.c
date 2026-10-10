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
#include "renderer/debug.h"
#include "renderer/renderer.h"
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
	const char* modelName = cJSON_GetObjectItem(item, "model")->valuestring;
	const char* textureName = cJSON_GetObjectItem(item, "mat")->valuestring;

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

	cJSON* transform = jsonGetObjectC(item, "transform");
	def->positionOffset[0] = 0;
	def->positionOffset[1] = 0;
	def->positionOffset[2] = 0;

	def->rotationOffset[0] = 0;
	def->rotationOffset[1] = 0;
	def->rotationOffset[2] = 0;
	if (transform)
	{
		cJSON* position = cJSON_GetObjectItem(transform, "position");
		cJSON* rotation = cJSON_GetObjectItem(transform, "rotation");
		if (position)
			jsonGetVec3(transform, "position", def->positionOffset);
		if (rotation)
			jsonGetVec3(transform, "rotation", def->rotationOffset);
		jsonPop();
	}

	cJSON* editorTransformJson = jsonGetObjectC(item, "editorTransform");
	mat4 editorTransform;
	def->editorPos[0] = 0;
	def->editorPos[1] = 0;
	def->editorPos[2] = 0;
	def->editorRot[0] = 0;
	def->editorRot[1] = 0;
	def->editorRot[2] = 0;
	glm_mat4_identity(editorTransform);
	if (editorTransformJson)
	{
		vec3 pos = {0, 0, 0};
		vec3 rot = {0, 0, 0};
		cJSON* position = cJSON_GetObjectItem(editorTransformJson, "position");
		cJSON* rotation = cJSON_GetObjectItem(editorTransformJson, "rotation");
		if (position)
			jsonGetVec3(editorTransformJson, "position", pos);
		if (rotation)
			jsonGetVec3(editorTransformJson, "rotation", rot);
		jsonPop();

		memcpy(def->editorPos, pos, sizeof(vec3));
		memcpy(def->editorRot, rot, sizeof(vec3));

		mat4 rotMat;
		vec4 quat;
		rot[0] = glm_rad(rot[0]);
		rot[1] = glm_rad(rot[1]);
		rot[2] = glm_rad(rot[2]);
		glm_translate(editorTransform, pos);
		glm_euler_yzx_quat(rot, quat);
		glm_quat_mat4(quat, rotMat);
		glm_mat4_mul(editorTransform, rotMat, editorTransform);
	}
	memcpy(def->editorTransform, editorTransform, sizeof(mat4));

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
	drawMesh(def->mesh, def->texture, transform);
	return;

	vec3 forward = {0, 1, 0};
	vec3 left = {0, 0, 1};
	vec3 up = {1, 0, 0};

	//
	// Extract the basis vectors from the matrix. Since we only need the Z
	// component of the up vector, we don't get X and Y.
	//
	vec4 quat;
	memcpy(quat, item->quat, sizeof(vec4));
	glm_quat_mat4(quat, transform);
	glm_mat4_mulv3(transform, forward, 1, forward);
	glm_mat4_mulv3(transform, left, 1, left);
	glm_mat4_mulv3(transform, up, 1, up);

	// drawDebugPoint(forward, item->pos, 0);
	// drawDebugPoint(up, item->pos, 1);
	// drawDebugPoint(left, item->pos, 2);

	vec3 out;

	vec3 f;
	f[0] = transform[2][0];
	f[1] = transform[2][1];
	f[2] = transform[2][2];

	float xyDist = sqrtf( f[2] * f[2] + f[1] * f[1] );
	out[1] = atan2(f[0], f[2]);
	out[2] = atan2(-f[1], xyDist);

	printf("pitch %.2f\n", glm_deg(out[2]));
	printf("xyDist %.2f\n", xyDist);

	drawDebugPoint(f, item->pos, 2);

	// float xyDist = sqrtf( forward[0] * forward[0] + forward[2] * forward[2] );
	//
	//// enough here to get angles?
	// if ( xyDist > 0.001f )
	//{
	//	// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
	//	out[1] = ( atan2f( forward[2], forward[0] ) );

	//	// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
	//	out[2] = ( atan2f( -forward[1], xyDist ) );

	//	// (roll)	z = ATAN( left.z, up.z );
	//	out[0] = ( atan2f( left[1], up[1] ) );
	//}
	// else	// forward is mostly Z, gimbal lock-
	//{
	//	// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
	//	out[1] = ( atan2f( -left[0], left[1] ) );

	//	// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
	//	out[2] = ( atan2f( -forward[1], xyDist ) );

	//	// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
	//	out[0] = 0;
	//}
}

void entityItemExport(Item* item)
{
	if (item->index == 10)
		printf("help\n");
	EntityItemDef* defData = item->def->data;

	char buf[100];
	snprintf(buf, 100, "%s%d", item->def->name, item->index);

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
	memcpy(entPos, defData->positionOffset, sizeof(vec3));
	
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
	glm_quat_norm(itemRot);
	glm_quat_norm(quat);
	glm_quat_mul(itemRot, quat, newRot);
	glm_quat_norm(newRot);
	glm_quat_mat4(newRot, rotMat);

	// vec3 eye = {0, 0, 0};
	// vec3 up = {0, 1, 0};
	// vec3 forward = {-1, 0, 0};
	// glm_mat4_mulv3(rotMat, up, 1, up);
	// glm_mat4_mulv3(rotMat, forward, 1, forward);
	// glm_lookat(eye, forward, up, rotMat);

	getEulerAngles(rotMat, entRot);
	//entRot[0] = asin(-rotMat[0][1]);
	//entRot[1] = atan2(rotMat[2][1], rotMat[1][1]);
	//entRot[2] = atan2(rotMat[0][2], rotMat[0][0]);
	entRot[0] = glm_deg(entRot[0]);
	entRot[1] = glm_deg(entRot[1]);
	entRot[2] = glm_deg(entRot[2]);
	memcpy(entity->rotation, entRot, sizeof(vec3));

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
