#pragma once

#include "item/item.h"
#include "renderer/texture.h"

#include <cjson.h>
#include <cglm/cglm.h>

#define PANEL_ITEM_ID_TOP 0
#define PANEL_ITEM_ID_BOTTOM 1
#define PANEL_ITEM_ID_LEFT 2
#define PANEL_ITEM_ID_RIGHT 3

#define PANEL_ITEM_ID_TOP_LEFT 4
#define PANEL_ITEM_ID_TOP_RIGHT 5
#define PANEL_ITEM_ID_BOTTOM_LEFT 6
#define PANEL_ITEM_ID_BOTTOM_RIGHT 7

typedef struct
{
  mat4 transform;
  mat4 editorTransform;
  const char* model;
  Mesh* mesh;
  vec3 pos;
  vec3 rot;
  vec3 editorPos;
  vec3 editorRot;
} Prop;

typedef struct
{
  Texture editorTexture;
  Texture texture;
	float minSize;
	float maxSize;
} PanelItemDef;

typedef struct
{
  PanelItemDef items[8];
  Prop props[8];

  Texture editorCenterTexture;
  Texture centerTexture;
  Texture zTexture;

  int exportThickness;
  const char* classname;

  vec2 minSize;
  vec2 maxSize;
  vec2 defaultSize;
  char horizTile;
  char vertTile;
} PanelDefData;

typedef struct
{
	vec2 size;
	ivec2 tile;
} PanelData;

void* loadPanelItemDef(cJSON* json, ItemDefinition* def);
void panelItemInit(Item* item);
void panelItemExport(Item* item);
void panelItemRender(Item* item);
void panelItemSave(Item* item, cJSON* json);
void panelItemLoad(Item* item, cJSON* json);
void panelItemGetBoundingBox(Item* item, vec3 min, vec3 max);
