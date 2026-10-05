#include "overlayItem.h"
#include "export/overlay.h"
#include "renderer/renderer.h"
#include "assetManager.h"
#include "jsonUtils.h"
#include <string.h>

static ItemCallbacks callbacks;

static __attribute__((constructor)) void init()
{
	callbacks.init = overlayItemInit;
	callbacks.exportItem = overlayItemExport;
	callbacks.render = overlayItemRender;
	callbacks.getBoundingBox = overlayItemGetBoundingBox;
	callbacks.save = overlayItemSave;
	callbacks.load = overlayItemLoad;
}

void* loadOverlayItemDef(cJSON* item, ItemDefinition* itemDef)
{
	itemDef->callbacks = &callbacks;
	OverlayItemDef* def = malloc(sizeof(OverlayItemDef));
	const char* textureName = cJSON_GetObjectItem(item, "texture")->valuestring;
	def->texture = assetManagerLoadTexture(textureName);
	def->exportTexture = jsonGetStr(item, "exportTexture");
	jsonGetVec2(item, "size", def->size);
	return def;
}

void overlayItemInit(Item* item)
{
}
void overlayItemExport(Item* item)
{
	char buf[100];
	snprintf(buf, 100, "%s%d", item->def->name, item->index);

	OverlayItemDef* def = item->def->data;
	Overlay* overlay = exportCreateOverlay();
	overlay->name = strdup(buf);
	overlay->texture = def->exportTexture;
	memcpy(overlay->pos, item->pos, sizeof(vec3));
	memcpy(overlay->rotation, item->dir, sizeof(vec3));
	memcpy(overlay->size, def->size, sizeof(vec3));
	overlay->tile[0] = 1;
	overlay->tile[1] = 1;
	overlay->tint[0] = 255;
	overlay->tint[1] = 255;
	overlay->tint[2] = 255;
	overlay->tint[3] = 255;
}
void overlayItemRender(Item* item)
{
	OverlayItemDef* def = item->def->data;
	vec2 start = {-def->size[0] / 128.0f, -def->size[1] / 128.0f};
	vec2 end = {def->size[0] / 128.0f, def->size[1] / 128.0f};
	overlayDrawRect(start, end, def->texture);
	panelEndFrame(item->transform, 1);
}
void overlayItemGetBoundingBox(Item* item, vec3 min, vec3 max)
{
	OverlayItemDef* def = item->def->data;
	min[0] = -def->size[0] / 128.0f;
	min[1] = -0.125;
	min[2] = -def->size[1] / 128.0f;
	max[0] = def->size[0] / 128.0f;
	max[1] = 0.125;
	max[2] = def->size[1] / 128.0f;
}
void overlayItemSave(Item* item, cJSON* json)
{
}
void overlayItemLoad(Item* item, cJSON* json)
{
}
