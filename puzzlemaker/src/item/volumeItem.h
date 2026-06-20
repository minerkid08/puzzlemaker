#include "cglm/types.h"
#include "cjson.h"
#include "item/item.h"

typedef struct
{
	unsigned int material;
	const char* exportMaterial;
	const char* entity;
	vec3 minSize;
	vec3 maxSize;
	vec3 defaultSize;
} VolumeItemDef;

typedef struct
{
	vec3 size;
} VolumeItemData;

void* loadVolumeItemDef(cJSON* json);
void volumeItemInit(Item* item);
void volumeItemExport(Item* item);
void volumeItemRender(Item* item);
void volumeItemSave(Item* item, cJSON* json);
void volumeItemLoad(Item* item, cJSON* json);
