#include "cjson.h"
#include "item/item.h"
#include "renderer/texture.h"

typedef struct
{
	const char* exportTexture;
	vec2 size;
	Texture texture;
} OverlayItemDef;

void* loadOverlayItemDef(cJSON* json, ItemDefinition* def);
void overlayItemInit(Item* item);
void overlayItemExport(Item* item);
void overlayItemRender(Item* item);
void overlayItemGetBoundingBox(Item* item, vec3 min, vec3 max);
void overlayItemSave(Item* item, cJSON* json); 
void overlayItemLoad(Item* item, cJSON* json); 
