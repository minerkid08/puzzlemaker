#include "overlayItem.h"
#include "export/overlay.h"
#include "item.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "renderer/renderer.h"
#include "utils.h"
#include <string.h>

static ItemCallbacks callbacks;

#ifdef __linux__
__attribute__((constructor)) static void overlayItemProcInit()
#else
void overlayItemProcInit()
#endif
{
	callbacks.init = overlayItemInit;
	callbacks.exportItem = overlayItemExport;
	callbacks.render = overlayItemRender;
	callbacks.getBoundingBox = overlayItemGetBoundingBox;
	callbacks.save = overlayItemSave;
	callbacks.load = overlayItemLoad;
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*overlayItemProcInitPtr)(void) = overlayItemProcInit;

#pragma comment(linker, "/include:overlayItemProcInitPtr")
#endif

void* loadOverlayItemDef(cJSON* item, ItemDefinition* itemDef)
{
	itemDef->callbacks = &callbacks;
	OverlayItemDef* def = malloc(sizeof(OverlayItemDef));

	char flags = 0;
	jsonGetEditorTex(item, "editorTexture", &def->texture, &flags);
	itemDef->flags |= flags;

	if ((itemDef->flags & ITEM_FLAGS_NODEFAULTEXPORT) == 0)
		def->exportTexture = jsonGetStrD(item, "texture");
	jsonGetVec2(item, "size", def->size);
	return def;
}

void overlayItemInit(Item* item)
{
	item->data = 0;
}

void overlayItemExport(Item* item)
{
	OverlayItemDef* def = item->def->data;

	char* str = itemProcessString(def->exportTexture, item->kv);

	Overlay* overlay = exportCreateOverlay();
	overlay->texture = str;
	overlay->script = 1;
	char buf[256];
	snprintf(buf, 256, "%s%d", item->def->id, item->index);

	overlay->name = strdup(buf);
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
	setTransform(item->transform);
	OverlayItemDef* def = item->def->data;
	vec3 start = {-def->size[0] / 128.0f, -def->size[1] / 128.0f, 0.01};
	vec2 size = {def->size[0] / 64.0f, def->size[1] / 64.0f};

	vec3 verts[4];
	genVerts(DIR_POS_Z, start, size, verts);

	drawRect(verts, &def->texture);
}
void overlayItemGetBoundingBox(Item* item, vec3 min, vec3 max)
{
	OverlayItemDef* def = item->def->data;
	min[0] = -def->size[0] / 128.0f;
	min[1] = -def->size[1] / 128.0f;
	min[2] = -0.125;
	max[0] = def->size[0] / 128.0f;
	max[1] = def->size[1] / 128.0f;
	max[2] = 0.125;
}
void overlayItemSave(Item* item, cJSON* json)
{
}
void overlayItemLoad(Item* item, cJSON* json)
{
}
