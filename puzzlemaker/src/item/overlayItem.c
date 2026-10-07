#include "overlayItem.h"
#include "assetManager.h"
#include "dynList.h"
#include "export/overlay.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "renderer/renderer.h"
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

char startsWith(const char* a, const char* b)
{
	int l = strlen(b);
	for (int i = 0; i < l; i++)
	{
		if (a[i] == 0)
			return 0;
		if (a[i] != b[i])
			return 0;
	}
	return 1;
}

void overlayItemExport(Item* item)
{
	OverlayItemDef* def = item->def->data;
	char buf[256];
	int len = strlen(def->exportTexture);
	int k = 0;
	for (int i = 0; i < len; i++)
	{
		char c = def->exportTexture[i];
		if (c == '{')
		{
			int l = dynList_size(item->def->kvs);
			for (int j = 0; j < l; j++)
			{
				const char* b = item->def->kvs[j].name;
				if (startsWith(def->exportTexture + i + 1, b))
				{
					int t = item->def->kvs[j].type;
					V* values = item->def->kvs[j].dropValues;
					int v = item->kv[j].value.i;
					if (t == (TYPE_DROPDOWN | TYPE_STRING))
					{
						i += strlen(b);
						b = values[v].s;
						strcpy(buf + k, b);
						k += strlen(b);
					}
					if (t == (TYPE_DROPDOWN | TYPE_INT))
					{
						i += strlen(b);
						int m = sprintf(buf + k, "%d", values[v].i);
						k += m;
					}
					i++;
					break;
				}
			}
		}
		else
			buf[k++] = c;
	}
	buf[k] = 0;

	Overlay* overlay = exportCreateOverlay();
	overlay->texture = strdup(buf);
	overlay->script = 1;
	snprintf(buf, 256, "%s%d", item->def->name, item->index);

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
	OverlayItemDef* def = item->def->data;
	vec2 start = {-def->size[0] / 128.0f, -def->size[1] / 128.0f};
	vec2 end = {def->size[0] / 128.0f, def->size[1] / 128.0f};
	overlayDrawRect(start, end, def->texture, 1);
	panelEndFrame(item->transform, 1, 1);
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
