#include "item/panel.h"
#include "assetManager.h"
#include "cglm/mat4.h"
#include "cjson.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "jsonUtils.h"
#include "renderer/renderer.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void* loadPanelItemDef(cJSON* item)
{
	PanelDefData* data = malloc(sizeof(PanelDefData));

	if (cJSON_GetObjectItem(item, "minSize"))
		jsonGetVec2(item, "minSize", data->minSize);
	else
	{
		data->minSize[0] = 0;
		data->minSize[1] = 0;
	}
	if (cJSON_GetObjectItem(item, "maxSize"))
		jsonGetVec2(item, "maxSize", data->maxSize);
	else
	{
		data->maxSize[0] = 99999;
		data->maxSize[1] = 99999;
	}
	if (cJSON_GetObjectItem(item, "defaultSize"))
		jsonGetVec2(item, "defaultSize", data->defaultSize);
	else
	{
		data->defaultSize[0] = 4;
		data->defaultSize[1] = 4;
	}

	if (cJSON_GetObjectItem(item, "horizTile"))
		data->horizTile = jsonGetBool(item, "horizTile");
	else
		data->horizTile = 0;

	if (cJSON_GetObjectItem(item, "vertTile"))
		data->vertTile = jsonGetBool(item, "vertTile");
	else
		data->vertTile = 0;

	for (int i = 0; i < 8; i++)
	{
		data->items[i].material = 0;
		data->items[i].maxSize = 0;
		data->items[i].minSize = 0;
	}

	const char* filename = cJSON_GetObjectItem(item, "editorCenterTexture")->valuestring;
	data->centerMat = assetManagerLoadTexture(filename);

	cJSON* arr = cJSON_GetObjectItem(item, "boarders");
	cJSON* boarder;
	cJSON_ArrayForEach(boarder, arr)
	{
		int index = 0;
		const char* id = cJSON_GetObjectItem(boarder, "id")->valuestring;
		if (strcmp(id, "top") == 0)
			index = PANEL_ITEM_ID_TOP;
		else if (strcmp(id, "bottom") == 0)
			index = PANEL_ITEM_ID_BOTTOM;
		else if (strcmp(id, "left") == 0)
			index = PANEL_ITEM_ID_LEFT;
		else if (strcmp(id, "right") == 0)
			index = PANEL_ITEM_ID_RIGHT;
		else if (strcmp(id, "top-left") == 0)
			index = PANEL_ITEM_ID_TOP_LEFT;
		else if (strcmp(id, "top-right") == 0)
			index = PANEL_ITEM_ID_TOP_RIGHT;
		else if (strcmp(id, "bottom-left") == 0)
			index = PANEL_ITEM_ID_BOTTOM_LEFT;
		else if (strcmp(id, "bottom-right") == 0)
			index = PANEL_ITEM_ID_BOTTOM_RIGHT;
		else
			errorf("bad id '%s'\n", id);

		PanelItemDef* item = &data->items[index];
		const char* filename = cJSON_GetObjectItem(boarder, "editorTexture")->valuestring;
		item->material = assetManagerLoadTexture(filename);

		item->maxSize = 9999;
		if (cJSON_GetObjectItem(boarder, "maxSize"))
			item->maxSize = jsonGetFloat(boarder, "maxSize");

		item->minSize = 0;
		if (cJSON_GetObjectItem(boarder, "minSize"))
			item->minSize = jsonGetFloat(boarder, "minSize");

		item->exportMaterial = jsonGetStr(boarder, "texture");
		item->texSize = jsonGetInt(boarder, "texSize");
	}

	float size1 = data->items[PANEL_ITEM_ID_RIGHT].minSize;
	float size2 = data->items[PANEL_ITEM_ID_LEFT].minSize;
	if (data->minSize[0] < size1 + size2)
		data->minSize[0] = size1 + size2;

	size1 = data->items[PANEL_ITEM_ID_TOP].minSize;
	size2 = data->items[PANEL_ITEM_ID_BOTTOM].minSize;
	if (data->minSize[1] < size1 + size2)
		data->minSize[1] = size1 + size2;

	data->exportCetnerMat = jsonGetStr(item, "centerTexture");
	data->exportZMat = jsonGetStr(item, "zTexture");
	data->exportThickness = jsonGetInt(item, "thickness");
	data->texSize = jsonGetInt(item, "texSize");

	if (cJSON_GetObjectItem(item, "entity"))
		data->classname = jsonGetStr(item, "entity");
	else
		data->classname = 0;

	return data;
}

void panelItemInit(Item* item)
{
	PanelDefData* def = item->def->data;
	PanelData* data = malloc(sizeof(PanelData));
	data->size[0] = def->defaultSize[0];
	data->size[1] = def->defaultSize[1];
	item->data = data;
}

typedef struct
{
	float topSize;
	float bottomSize;
	float leftSize;
	float rightSize;
	float centerWidth;
	float centerHeight;
} PanelSizes;

void getPanelSizes(Item* item, PanelSizes* sizes)
{

	PanelData* data = item->data;
	PanelDefData* def = item->def->data;
	PanelItemDef* boarder = def->items;

	sizes->topSize = boarder[PANEL_ITEM_ID_TOP].maxSize;
	sizes->bottomSize = boarder[PANEL_ITEM_ID_BOTTOM].maxSize;
	sizes->leftSize = boarder[PANEL_ITEM_ID_LEFT].maxSize;
	sizes->rightSize = boarder[PANEL_ITEM_ID_RIGHT].maxSize;

	if (data->size[0] < sizes->leftSize + sizes->rightSize)
	{
		float t = sizes->leftSize + sizes->rightSize;

		if (sizes->rightSize == 0)
			sizes->leftSize = data->size[0];
		else
		{
			float r = sizes->leftSize / sizes->rightSize;

			sizes->leftSize = data->size[0] * r;
			if (sizes->leftSize < boarder[PANEL_ITEM_ID_LEFT].minSize)
			{
				sizes->leftSize = boarder[PANEL_ITEM_ID_LEFT].minSize;
				sizes->rightSize = data->size[0] - sizes->leftSize;
			}
			else
				sizes->rightSize = data->size[0] * (1 - r);

			if (sizes->rightSize < boarder[PANEL_ITEM_ID_RIGHT].minSize)
			{
				sizes->rightSize = boarder[PANEL_ITEM_ID_RIGHT].minSize;
				sizes->leftSize = data->size[0] - sizes->rightSize;
			}
		}
	}
	else
		sizes->centerWidth = data->size[0] - sizes->leftSize - sizes->rightSize;
	if (data->size[1] < sizes->topSize + sizes->bottomSize)
	{
		float t = sizes->topSize + sizes->bottomSize;

		if (sizes->bottomSize == 0)
			sizes->topSize = data->size[1];
		else
		{
			float r = sizes->topSize / sizes->bottomSize;

			sizes->topSize = data->size[1] * r;
			if (sizes->topSize < boarder[PANEL_ITEM_ID_TOP].minSize)
			{
				sizes->topSize = boarder[PANEL_ITEM_ID_TOP].minSize;
				sizes->bottomSize = data->size[1] - sizes->topSize;
			}
			else
				sizes->bottomSize = data->size[1] * (1 - r);

			if (sizes->bottomSize < boarder[PANEL_ITEM_ID_BOTTOM].minSize)
			{
				sizes->bottomSize = boarder[PANEL_ITEM_ID_BOTTOM].minSize;
				sizes->topSize = data->size[0] - sizes->bottomSize;
			}
		}
	}
	else
		sizes->centerHeight = data->size[1] - sizes->topSize - sizes->bottomSize;
}

void panelItemRender(Item* item)
{
	PanelData* data = item->data;
	PanelDefData* def = item->def->data;
	PanelItemDef* boarder = def->items;

	PanelSizes sizes;

	getPanelSizes(item, &sizes);

	vec2 start;
	vec2 end;

	if (sizes.leftSize > 0 && sizes.bottomSize > 0)
	{
		start[0] = 0;
		start[1] = 0;
		end[0] = sizes.leftSize;
		end[1] = sizes.bottomSize;
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_BOTTOM_LEFT].material);
	}

	if (sizes.leftSize > 0 && sizes.centerHeight > 0)
	{
		start[0] = 0;
		start[1] = sizes.bottomSize;
		end[0] = sizes.leftSize;
		end[1] = sizes.bottomSize + sizes.centerHeight;
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_LEFT].material);
	}

	if (sizes.leftSize > 0 && sizes.topSize > 0)
	{
		start[0] = 0;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = sizes.leftSize;
		end[1] = data->size[1];
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_TOP_LEFT].material);
	}

	if (sizes.centerWidth > 0 && sizes.bottomSize > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = 0;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = sizes.bottomSize;
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_BOTTOM].material);
	}

	if (sizes.centerWidth > 0 && sizes.centerHeight > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = sizes.bottomSize;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = sizes.bottomSize + sizes.centerHeight;
		panelDrawRect(start, end, def->centerMat);
	}

	if (sizes.centerWidth > 0 && sizes.topSize > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = data->size[1];
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_TOP].material);
	}

	if (sizes.rightSize > 0 && sizes.bottomSize > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = 0;
		end[0] = data->size[0];
		end[1] = sizes.bottomSize;
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_BOTTOM_RIGHT].material);
	}

	if (sizes.rightSize > 0 && sizes.centerHeight > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = sizes.bottomSize;
		end[0] = data->size[0];
		end[1] = sizes.bottomSize + sizes.centerHeight;
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_RIGHT].material);
	}

	if (sizes.rightSize > 0 && sizes.topSize > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = data->size[0];
		end[1] = data->size[1];
		panelDrawRect(start, end, boarder[PANEL_ITEM_ID_TOP_RIGHT].material);
	}

	panelEndFrame(item->transform, 0);
}

static void addBrush(Entity* entity, vec3 start, vec3 end, const char* mat, const char* zTex, mat4 transform,
					 int texSize, const char* altTex, int altSize)
{
	Brush* brush = exportCreateBrush(start, end);
	brush->ent = 1;
	exportEntityAddBrush(entity, brush);
	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];

		for (int j = 0; j < 4; j++)
		{
			vec3 res;
			vec3 vert;
			memcpy(vert, side->verts[j], sizeof(vec3));
			glm_mat4_mulv3(transform, vert, 1, res);
			memcpy(side->verts[j], res, sizeof(vec3));
		}
		if (side->id == DIR_POS_Z || (side->id == DIR_NEG_Z && altTex == 0))
		{
			side->material = mat;
			side->fit = 1;
			side->texWidth = texSize;
			side->texHeight = texSize;
		}
		else if (side->id == DIR_NEG_Z && altTex)
		{
			side->material = altTex;
			side->fit = 1;
			side->texWidth = altSize;
			side->texHeight = altSize;
		}
		else
			side->material = zTex;
	}
}

void panelItemSave(Item* item, cJSON* json)
{
	PanelData* data = item->data;
	cJSON* obj = cJSON_CreateObject();
	cJSON_AddNumberToObject(obj, "x", data->size[0]);
	cJSON_AddNumberToObject(obj, "y", data->size[1]);
	cJSON_AddItemToObject(json, "size", obj);
}

void panelItemLoad(Item* item, cJSON* json)
{
	PanelData* data = item->data;
	cJSON* obj = cJSON_GetObjectItem(json, "size");
	data->size[0] = jsonGetFloat(obj, "x");
	data->size[1] = jsonGetFloat(obj, "y");
}

void panelItemExport(Item* item)
{
	PanelData* data = item->data;
	PanelDefData* defData = item->def->data;
	PanelItemDef* boarder = defData->items;

	Entity* entity = exportCreateEntity();

	char buf[20];
	snprintf(buf, 20, "%s%d", item->def->name, item->index);
	entity->name = strdup(buf);
	entity->className = defData->classname;

	memcpy(entity->pos, item->pos, sizeof(vec3));
	memcpy(entity->rotation, item->dir, sizeof(vec3));

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

	vec3 start;
	vec3 end;
	start[0] = 0;
	start[1] = 0;
	start[2] = defData->exportThickness / (2.0f * 64.0f);

	end[0] = data->size[0];
	end[1] = data->size[1];
	end[2] = -defData->exportThickness / (2.0f * 64.0f);

	mat4 transform;
	memcpy(transform, item->transform, sizeof(mat4));

	PanelSizes sizes;

	getPanelSizes(item, &sizes);

	const char* zTex = defData->exportZMat;

	if (sizes.leftSize > 0 && sizes.bottomSize > 0)
	{
		start[0] = 0;
		start[1] = 0;
		end[0] = sizes.leftSize;
		end[1] = sizes.bottomSize;
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_BOTTOM_LEFT];
		PanelItemDef* b2 = &boarder[PANEL_ITEM_ID_BOTTOM_RIGHT];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, b2->exportMaterial, b2->texSize);
	}

	if (sizes.leftSize > 0 && sizes.centerHeight > 0)
	{
		start[0] = 0;
		start[1] = sizes.bottomSize;
		end[0] = sizes.leftSize;
		end[1] = sizes.bottomSize + sizes.centerHeight;
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_LEFT];
		PanelItemDef* b2 = &boarder[PANEL_ITEM_ID_RIGHT];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, b2->exportMaterial, b2->texSize);
	}

	if (sizes.leftSize > 0 && sizes.topSize > 0)
	{
		start[0] = 0;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = sizes.leftSize;
		end[1] = data->size[1];
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_TOP_LEFT];
		PanelItemDef* b2 = &boarder[PANEL_ITEM_ID_TOP_RIGHT];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, b2->exportMaterial, b2->texSize);
	}

	if (sizes.centerWidth > 0 && sizes.bottomSize > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = 0;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = sizes.bottomSize;
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_BOTTOM];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, 0, 0);
	}

	if (sizes.centerWidth > 0 && sizes.centerHeight > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = sizes.bottomSize;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = sizes.bottomSize + sizes.centerHeight;
		addBrush(entity, start, end, defData->exportCetnerMat, zTex, transform, defData->texSize, 0, 0);
	}

	if (sizes.centerWidth > 0 && sizes.topSize > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = data->size[1];
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_TOP];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, 0, 0);
	}

	if (sizes.rightSize > 0 && sizes.bottomSize > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = 0;
		end[0] = data->size[0];
		end[1] = sizes.bottomSize;
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_BOTTOM_RIGHT];
		PanelItemDef* b2 = &boarder[PANEL_ITEM_ID_BOTTOM_LEFT];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, b2->exportMaterial, b2->texSize);
	}

	if (sizes.rightSize > 0 && sizes.centerHeight > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = sizes.bottomSize;
		end[0] = data->size[0];
		end[1] = sizes.bottomSize + sizes.centerHeight;
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_RIGHT];
		PanelItemDef* b2 = &boarder[PANEL_ITEM_ID_LEFT];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, b2->exportMaterial, b2->texSize);
	}

	if (sizes.rightSize > 0 && sizes.topSize > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = data->size[0];
		end[1] = data->size[1];
		PanelItemDef* b = &boarder[PANEL_ITEM_ID_TOP_RIGHT];
		PanelItemDef* b2 = &boarder[PANEL_ITEM_ID_TOP_LEFT];
		addBrush(entity, start, end, b->exportMaterial, zTex, transform, b->texSize, b2->exportMaterial, b2->texSize);
	}
}
