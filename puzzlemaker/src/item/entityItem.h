#pragma once

#include "cjson.h"
#include "item.h"

typedef struct
{
	const char* instanceName;
	const char* entityName;

	vec4 bound1;
	vec4 bound2;

	vec3 positionOffset;
	vec3 rotationOffset;
	mat4 editorTransform;

	Mesh* mesh;
	unsigned int texture;

} EntityItemDef;

void* loadEntityItemDef(cJSON* json, ItemDefinition* def);
void entityItemInit(Item* item);
void entityItemExport(Item* item);
void entityItemRender(Item* item);
void entityItemGetBoundingBox(Item* item, vec3 min, vec3 max);
void entityItemSave(Item* item, cJSON* json); 
void entityItemLoad(Item* item, cJSON* json); 
