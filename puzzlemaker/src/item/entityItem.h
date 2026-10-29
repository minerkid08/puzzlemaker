#pragma once

#include "cjson.h"
#include "item/item.h"

typedef struct
{
	mat4 editorTransform;

	const char* instanceName;
	const char* entityName;
	Mesh* mesh;

	vec4 bound1;
	vec4 bound2;

	vec3 positionOffset;
	vec3 rotationOffset;
	vec3 editorPos;
	vec3 editorRot;
} EntityItemDef;

void* loadEntityItemDef(cJSON* json, ItemDefinition* def);
void entityItemInit(Item* item);
void entityItemExport(Item* item);
void entityItemRender(Item* item);
void entityItemGetBoundingBox(Item* item, vec3 min, vec3 max);
void entityItemSave(Item* item, cJSON* json); 
void entityItemLoad(Item* item, cJSON* json); 
