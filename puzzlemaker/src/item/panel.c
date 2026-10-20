#include "item/panel.h"
#include "assetManager.h"
#include "cglm/mat4.h"
#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "export/brush.h"
#include "export/entity.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

static ItemCallbacks callbacks;

static __attribute__((constructor)) void init()
{
	callbacks.init = panelItemInit;
	callbacks.exportItem = panelItemExport;
	callbacks.render = panelItemRender;
	callbacks.getBoundingBox = panelItemGetBoundingBox;
	callbacks.save = panelItemSave;
	callbacks.load = panelItemLoad;
}

void* loadPanelItemDef(cJSON* item, ItemDefinition* itemDef)
{
	itemDef->callbacks = &callbacks;
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
		data->items[i].maxSize = 0;
		data->items[i].minSize = 0;
	}

  char flags = 0;
	jsonGetEditorTex(item, "editorCenterTexture", &data->editorCenterTexture, &flags);
  itemDef->flags |= flags;

	cJSON* arr = jsonGetArrayC(item, "borders");
	if (arr)
	{
		int size = cJSON_GetArraySize(arr);
		for (int i = 0; i < size; i++)
		{
			cJSON* border = jsonArrGetObject(arr, i);
			int index = 0;
			const char* id = jsonGetStr(border, "id");
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
				errorf("bad id '%s'", id);

			PanelItemDef* item = &data->items[index];
      char flags = 0;
			jsonGetEditorTex(border, "editorTexture", &item->editorTexture, &flags);
      itemDef->flags |= flags;

			item->maxSize = 9999;
			if (cJSON_GetObjectItem(border, "maxSize"))
				item->maxSize = jsonGetFloat(border, "maxSize");

			item->minSize = 0;
			if (cJSON_GetObjectItem(border, "minSize"))
				item->minSize = jsonGetFloat(border, "minSize");

			jsonGetTex(border, "texture", &item->texture);

			popStackFrame();
		}
		popStackFrame();
	}

	for (int i = 0; i < 8; i++)
		data->props[i].mesh = 0;

	cJSON* propArr = jsonGetArrayC(item, "props");
	if (propArr)
	{
		int size = cJSON_GetArraySize(propArr);
		for (int i = 0; i < size; i++)
		{
			cJSON* propJson = jsonArrGetObject(propArr, i);
			int index = 0;
			const char* id = jsonGetStr(propJson, "id");
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
				errorf("bad id '%s'", id);

			Prop* prop = &data->props[index];

			prop->model = jsonGetStr(propJson, "model");
			const char* meshName = jsonGetStr(propJson, "editorModel");
			prop->mesh = assetManagerLoadMesh(meshName);
			const char* textureName = jsonGetStr(propJson, "editorTexture");
      char flags = 0;
			prop->tex = assetManagerLoadTexture(textureName, &flags);
      itemDef->flags |= flags;

			jsonGetTransform(propJson, "transform", prop->pos, prop->rot, prop->transform);
			jsonGetTransform(propJson, "editorTransform", prop->editorPos, prop->editorRot, prop->editorTransform);

			popStackFrame();
		}
		popStackFrame();
	}

	float size1 = data->items[PANEL_ITEM_ID_RIGHT].minSize;
	float size2 = data->items[PANEL_ITEM_ID_LEFT].minSize;
	if (data->minSize[0] < size1 + size2)
		data->minSize[0] = size1 + size2;

	size1 = data->items[PANEL_ITEM_ID_TOP].minSize;
	size2 = data->items[PANEL_ITEM_ID_BOTTOM].minSize;
	if (data->minSize[1] < size1 + size2)
		data->minSize[1] = size1 + size2;

	jsonGetTex(item, "centerTexture", &data->centerTexture);
	jsonGetTex(item, "zTexture", &data->zTexture);
	data->exportThickness = jsonGetInt(item, "thickness");

	if (cJSON_GetObjectItem(item, "entity"))
		data->classname = jsonGetStrD(item, "entity");
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
	data->tile[0] = 1;
	data->tile[1] = 1;
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
	PanelItemDef* border = def->items;

	sizes->topSize = border[PANEL_ITEM_ID_TOP].maxSize;
	sizes->bottomSize = border[PANEL_ITEM_ID_BOTTOM].maxSize;
	sizes->leftSize = border[PANEL_ITEM_ID_LEFT].maxSize;
	sizes->rightSize = border[PANEL_ITEM_ID_RIGHT].maxSize;

	if (data->size[0] < sizes->leftSize + sizes->rightSize)
	{
		float t = sizes->leftSize + sizes->rightSize;

		if (sizes->rightSize == 0)
			sizes->leftSize = data->size[0];
		else
		{
			float r = sizes->leftSize / sizes->rightSize;

			sizes->leftSize = data->size[0] * r;
			if (sizes->leftSize < border[PANEL_ITEM_ID_LEFT].minSize)
			{
				sizes->leftSize = border[PANEL_ITEM_ID_LEFT].minSize;
				sizes->rightSize = data->size[0] - sizes->leftSize;
			}
			else
				sizes->rightSize = data->size[0] * (1 - r);

			if (sizes->rightSize < border[PANEL_ITEM_ID_RIGHT].minSize)
			{
				sizes->rightSize = border[PANEL_ITEM_ID_RIGHT].minSize;
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
			if (sizes->topSize < border[PANEL_ITEM_ID_TOP].minSize)
			{
				sizes->topSize = border[PANEL_ITEM_ID_TOP].minSize;
				sizes->bottomSize = data->size[1] - sizes->topSize;
			}
			else
				sizes->bottomSize = data->size[1] * (1 - r);

			if (sizes->bottomSize < border[PANEL_ITEM_ID_BOTTOM].minSize)
			{
				sizes->bottomSize = border[PANEL_ITEM_ID_BOTTOM].minSize;
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
	PanelItemDef* border = def->items;

	PanelSizes sizes;

	getPanelSizes(item, &sizes);

	vec2 start;
	vec2 end;

	for (int xTile = 0; xTile < data->tile[0]; xTile++)
	{
		float xOff = data->size[0] * xTile;
		for (int yTile = 0; yTile < data->tile[1]; yTile++)
		{
			float yOff = data->size[1] * yTile;

			if (sizes.leftSize > 0 && sizes.bottomSize > 0)
			{
				start[0] = xOff;
				start[1] = yOff;
				end[0] = sizes.leftSize + xOff;
				end[1] = sizes.bottomSize + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_BOTTOM_LEFT].editorTexture);
			}

			if (sizes.leftSize > 0 && sizes.centerHeight > 0)
			{
				start[0] = xOff;
				start[1] = sizes.bottomSize + yOff;
				end[0] = sizes.leftSize + xOff;
				end[1] = sizes.bottomSize + sizes.centerHeight + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_LEFT].editorTexture);
			}

			if (sizes.leftSize > 0 && sizes.topSize > 0)
			{
				start[0] = xOff;
				start[1] = sizes.bottomSize + sizes.centerHeight + yOff;
				end[0] = sizes.leftSize + xOff;
				end[1] = data->size[1] + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_TOP_LEFT].editorTexture);
			}

			if (sizes.centerWidth > 0 && sizes.bottomSize > 0)
			{
				start[0] = sizes.leftSize + xOff;
				start[1] = yOff;
				end[0] = sizes.leftSize + sizes.centerWidth + xOff;
				end[1] = sizes.bottomSize + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_BOTTOM].editorTexture);
			}

			if (sizes.centerWidth > 0 && sizes.centerHeight > 0)
			{
				start[0] = sizes.leftSize + xOff;
				start[1] = sizes.bottomSize + yOff;
				end[0] = sizes.leftSize + sizes.centerWidth + xOff;
				end[1] = sizes.bottomSize + sizes.centerHeight + yOff;
				panelDrawRect(start, end, &def->editorCenterTexture);
			}

			if (sizes.centerWidth > 0 && sizes.topSize > 0)
			{
				start[0] = sizes.leftSize + xOff;
				start[1] = sizes.bottomSize + sizes.centerHeight + yOff;
				end[0] = sizes.leftSize + sizes.centerWidth + xOff;
				end[1] = data->size[1] + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_TOP].editorTexture);
			}

			if (sizes.rightSize > 0 && sizes.bottomSize > 0)
			{
				start[0] = sizes.leftSize + sizes.centerWidth + xOff;
				start[1] = yOff;
				end[0] = data->size[0] + xOff;
				end[1] = sizes.bottomSize + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_BOTTOM_RIGHT].editorTexture);
			}

			if (sizes.rightSize > 0 && sizes.centerHeight > 0)
			{
				start[0] = sizes.leftSize + sizes.centerWidth + xOff;
				start[1] = sizes.bottomSize + yOff;
				end[0] = data->size[0] + xOff;
				end[1] = sizes.bottomSize + sizes.centerHeight + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_RIGHT].editorTexture);
			}

			if (sizes.rightSize > 0 && sizes.topSize > 0)
			{
				start[0] = sizes.leftSize + sizes.centerWidth + xOff;
				start[1] = sizes.bottomSize + sizes.centerHeight + yOff;
				end[0] = data->size[0] + xOff;
				end[1] = data->size[1] + yOff;
				panelDrawRect(start, end, &border[PANEL_ITEM_ID_TOP_RIGHT].editorTexture);
			}

			panelEndFrame(item->transform, 0, 1);

			for (int i = 0; i < 8; i++)
			{
				Prop* prop = &def->props[i];
				if (prop->mesh == 0)
					continue;
				vec3 pos;
				pos[2] = 0;
				switch (i)
				{
				case PANEL_ITEM_ID_BOTTOM_LEFT:
					pos[0] = 0;
					pos[1] = 0;
					break;
				case PANEL_ITEM_ID_BOTTOM:
					pos[0] = data->size[0] / 2.0f;
					pos[1] = 0;
					break;
				case PANEL_ITEM_ID_BOTTOM_RIGHT:
					pos[0] = data->size[0];
					pos[1] = 0;
					break;
				case PANEL_ITEM_ID_LEFT:
					pos[0] = 0;
					pos[1] = data->size[1] / 2.0f;
					break;
				case PANEL_ITEM_ID_RIGHT:
					pos[0] = data->size[0];
					pos[1] = data->size[1] / 2.0f;
					break;
				case PANEL_ITEM_ID_TOP_LEFT:
					pos[0] = 0;
					pos[1] = data->size[1];
					break;
				case PANEL_ITEM_ID_TOP:
					pos[0] = data->size[0] / 2.0f;
					pos[1] = data->size[1];
					break;
				case PANEL_ITEM_ID_TOP_RIGHT:
					pos[0] = data->size[0];
					pos[1] = data->size[1];
					break;
				}
				pos[0] += xOff;
				pos[1] += yOff;
				float tmp = pos[1];
				pos[1] = pos[2];
				pos[2] = tmp;

				mat4 transform;
				mat4 mat2;
				memcpy(transform, prop->editorTransform, sizeof(mat4));
				glm_mat4_identity(mat2);
				glm_translate(mat2, pos);
				glm_mat4_mul(mat2, transform, transform);
				memcpy(mat2, item->transform, sizeof(mat4));
				glm_mat4_mul(mat2, transform, transform);

				drawMesh(prop->mesh, prop->tex, transform);
			}
		}
	}
}

static void addBrush(Entity* entity, vec3 start, vec3 end, Texture* mat, Texture* zTex, Item* item, Texture* altTex)
{
	vec3 start2 = {start[0], start[2], start[1]};
	vec3 end2 = {end[0], end[2], end[1]};

	Brush* brush = exportCreateBrush(start2, end2);
	brush->pos[0] = item->pos[0];
	brush->pos[1] = item->pos[1];
	brush->pos[2] = item->pos[2];
	brush->rot[0] = item->dir[0];
	brush->rot[1] = item->dir[1];
	brush->rot[2] = item->dir[2];

	mat4 transform;
	memcpy(transform, item->transform, sizeof(mat4));

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];

		if (i == DIR_NEG_Y || (i == DIR_POS_Y && altTex == 0))
		{
			char fit = (mat->alignment == TEX_ALIGNMENT_FIT);
			side->material = mat->filename;
			side->fit = fit;
			side->texWidth = mat->size;
			side->texHeight = mat->size;
		}
		else if (i == DIR_POS_Y && altTex)
		{
			char fit = (altTex->alignment == TEX_ALIGNMENT_FIT);
			side->material = altTex->filename;
			side->fit = fit;
			side->texWidth = altTex->size;
			side->texHeight = altTex->size;
		}
		else
			side->material = zTex->filename;

		for (int j = 0; j < 4; j++)
		{
			vec3 res;
			vec3 vert;
			memcpy(vert, side->verts[j], sizeof(vec3));
			glm_mat4_mulv3(transform, vert, 1, res);
			memcpy(side->verts[j], res, sizeof(vec3));
		}
	}
	if (entity)
		exportEntityAddBrush(entity, brush);
}

void panelItemSave(Item* item, cJSON* json)
{
	PanelData* data = item->data;
	cJSON* size = cJSON_CreateObject();
	cJSON_AddNumberToObject(size, "x", data->size[0]);
	cJSON_AddNumberToObject(size, "y", data->size[1]);
	cJSON_AddItemToObject(json, "size", size);
	cJSON* tile = cJSON_CreateObject();
	cJSON_AddNumberToObject(tile, "x", data->tile[0]);
	cJSON_AddNumberToObject(tile, "y", data->tile[1]);
	cJSON_AddItemToObject(json, "tile", tile);
}

void panelItemLoad(Item* item, cJSON* json)
{
	PanelData* data = item->data;
	PanelDefData* def = item->def->data;

	cJSON* size = cJSON_GetObjectItem(json, "size");
	data->size[0] = jsonGetFloat(size, "x");
	data->size[1] = jsonGetFloat(size, "y");

	data->size[0] = fmaxf(data->size[0], def->minSize[0]);
	data->size[1] = fmaxf(data->size[1], def->minSize[1]);

	cJSON* tile = cJSON_GetObjectItem(json, "tile");
	if (tile)
	{
		data->tile[0] = jsonGetFloat(tile, "x");
		data->tile[1] = jsonGetFloat(tile, "y");
		data->tile[0] = max(data->tile[0], 1);
		data->tile[1] = max(data->tile[1], 1);
	}
}

void panelItemExport(Item* item)
{
	PanelData* data = item->data;
	PanelDefData* defData = item->def->data;
	PanelItemDef* border = defData->items;

	Entity* entity = 0;
	if (defData->classname)
	{
		entity = exportCreateEntity();

		char buf[20];
		snprintf(buf, 20, "%s%d", item->def->name, item->index);
		entity->name = strdup(buf);
		entity->className = defData->classname;

		memcpy(entity->pos, item->pos, sizeof(vec3));
		memset(entity->rotation, 0, sizeof(vec3));

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

	vec3 start;
	vec3 end;
	start[0] = 0;
	start[1] = 0;
	start[2] = defData->exportThickness / (2.0f * 64.0f);

	end[0] = data->size[0];
	end[1] = data->size[1];
	end[2] = -defData->exportThickness / (2.0f * 64.0f);

	PanelSizes sizes;

	getPanelSizes(item, &sizes);

	Texture* zTex = &defData->zTexture;

	if (sizes.leftSize > 0 && sizes.bottomSize > 0)
	{
		start[0] = 0;
		start[1] = 0;
		end[0] = sizes.leftSize;
		end[1] = sizes.bottomSize;
		PanelItemDef* b = &border[PANEL_ITEM_ID_BOTTOM_LEFT];
		PanelItemDef* b2 = &border[PANEL_ITEM_ID_BOTTOM_RIGHT];
		addBrush(entity, start, end, &b->texture, zTex, item, &b2->texture);
	}

	if (sizes.leftSize > 0 && sizes.centerHeight > 0)
	{
		start[0] = 0;
		start[1] = sizes.bottomSize;
		end[0] = sizes.leftSize;
		end[1] = sizes.bottomSize + sizes.centerHeight;
		PanelItemDef* b = &border[PANEL_ITEM_ID_LEFT];
		PanelItemDef* b2 = &border[PANEL_ITEM_ID_RIGHT];
		addBrush(entity, start, end, &b->texture, zTex, item, &b2->texture);
	}

	if (sizes.leftSize > 0 && sizes.topSize > 0)
	{
		start[0] = 0;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = sizes.leftSize;
		end[1] = data->size[1];
		PanelItemDef* b = &border[PANEL_ITEM_ID_TOP_LEFT];
		PanelItemDef* b2 = &border[PANEL_ITEM_ID_TOP_RIGHT];
		addBrush(entity, start, end, &b->texture, zTex, item, &b2->texture);
	}

	if (sizes.centerWidth > 0 && sizes.bottomSize > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = 0;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = sizes.bottomSize;
		PanelItemDef* b = &border[PANEL_ITEM_ID_BOTTOM];
		addBrush(entity, start, end, &b->texture, zTex, item, 0);
	}

	if (sizes.centerWidth > 0 && sizes.centerHeight > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = sizes.bottomSize;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = sizes.bottomSize + sizes.centerHeight;
		addBrush(entity, start, end, &defData->centerTexture, zTex, item, 0);
	}

	if (sizes.centerWidth > 0 && sizes.topSize > 0)
	{
		start[0] = sizes.leftSize;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = sizes.leftSize + sizes.centerWidth;
		end[1] = data->size[1];
		PanelItemDef* b = &border[PANEL_ITEM_ID_TOP];
		addBrush(entity, start, end, &b->texture, zTex, item, 0);
	}

	if (sizes.rightSize > 0 && sizes.bottomSize > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = 0;
		end[0] = data->size[0];
		end[1] = sizes.bottomSize;
		PanelItemDef* b = &border[PANEL_ITEM_ID_BOTTOM_RIGHT];
		PanelItemDef* b2 = &border[PANEL_ITEM_ID_BOTTOM_LEFT];
		addBrush(entity, start, end, &b->texture, zTex, item, &b2->texture);
	}

	if (sizes.rightSize > 0 && sizes.centerHeight > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = sizes.bottomSize;
		end[0] = data->size[0];
		end[1] = sizes.bottomSize + sizes.centerHeight;
		PanelItemDef* b = &border[PANEL_ITEM_ID_RIGHT];
		PanelItemDef* b2 = &border[PANEL_ITEM_ID_LEFT];
		addBrush(entity, start, end, &b->texture, zTex, item, &b2->texture);
	}

	if (sizes.rightSize > 0 && sizes.topSize > 0)
	{
		start[0] = sizes.leftSize + sizes.centerWidth;
		start[1] = sizes.bottomSize + sizes.centerHeight;
		end[0] = data->size[0];
		end[1] = data->size[1];
		PanelItemDef* b = &border[PANEL_ITEM_ID_TOP_RIGHT];
		PanelItemDef* b2 = &border[PANEL_ITEM_ID_TOP_LEFT];
		addBrush(entity, start, end, &b->texture, zTex, item, &b2->texture);
	}
}

void panelItemGetBoundingBox(Item* item, vec3 min, vec3 max)
{
	PanelData* data = item->data;

	min[0] = 0;
	min[1] = -0.125;
	min[2] = 0;
	max[0] = data->size[0] * data->tile[0];
	max[1] = 0.125;
	max[2] = data->size[1] * data->tile[1];
}
