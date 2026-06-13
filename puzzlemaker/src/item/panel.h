#pragma once

#include "item/item.h"

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
	unsigned int material;
  const char* exportMaterial;
  int texSize;
	float minSize;
	float maxSize;
} PanelItemDef;

typedef struct
{
  PanelItemDef items[8];
  unsigned int centerMat;

  const char* exportCetnerMat;
  const char* exportZMat;
  int exportThickness;
  int texSize;
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
} PanelData;

void* loadPanelItemDef(cJSON* json);
void panelItemInit(Item* item);
void panelItemExport(Item* item);
void panelItemRender(Item* item);
void panelItemSave(Item* item, cJSON* json);
void panelItemLoad(Item* item, cJSON* json);
