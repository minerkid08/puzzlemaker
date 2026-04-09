#pragma once

#include <cglm/cglm.h>

#define FaceRestriction_None (1 >> 0)
#define FaceRestriction_Wall (1 >> 1)
#define FaceRestriction_Floor (1 >> 2)
#define FaceRestriction_Ceiling (1 >> 3)

#define SnappingMode_Center (1 >> 0)
#define SnappingMode_Corner (1 >> 1)
#define SnappingMode_None (1 >> 2)

typedef struct
{
	const char* name;
	int faceRestrictions;
	int snappingMode;
	vec3 boundingBox;
	const char* material;
  unsigned int texture;
} ItemDefinition;

typedef struct
{
	int id;
	vec3 pos;
	vec3 dir;
} Item;

void initItems();

void drawItems();

void updateSelectedItem();
Item* findSelectedItem(vec3 pos, vec3 dir, float len);
Item* getSelectedItem();
void setSelectedItem(Item* item);
void startMove();
void endMove();

void addItemDef(ItemDefinition* def);
ItemDefinition* getItemDefinitions();

Item* addItem(int id);
void removeItem(int id);
