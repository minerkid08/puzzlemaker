#pragma once

#include "cjson.h"
#include "item.h"

typedef struct
{
	const char* instanceName;
	const char* entityName;

	Mesh* mesh;
	unsigned int texture;

} EntityItemDef;

void* loadEntityItemDef(cJSON* json);
void entityItemInit(Item* item);
void entityItemExport(Item* item);
void entityItemRender(Item* item);
