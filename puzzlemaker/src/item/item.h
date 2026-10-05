#pragma once

#include "cjson.h"
#include "renderer/mesh.h"
#include <cglm/cglm.h>

#define TYPE_BOOL 1
#define TYPE_INT 2
#define TYPE_STRING 3
#define TYPE_FLOAT 4
#define TYPE_PICKER 5
#define TYPE_INSTANCE 64
#define TYPE_DROPDOWN 128

#define ITEM_TYPE_ENTITY 1
#define ITEM_TYPE_PANEL 2
#define ITEM_TYPE_VOLUME 3
#define ITEM_TYPE_OVERLAY 4

#define SNAP_CORNER 0
#define SNAP_CENTER 1
#define SNAP_MINI_CORNER 2
#define SNAP_MINI_CENTER 3

typedef struct Item Item;

typedef union {
	int i;
	float f;
	char b;
	char* s;
} V;

typedef struct
{
	void (*init)(Item* item);
	void (*exportItem)(Item* item);
	void (*render)(Item* item);
	void (*getBoundingBox)(Item* item, vec3 min, vec3 max);
	void (*save)(Item* item, cJSON* json);
	void (*load)(Item* item, cJSON* json);
} ItemCallbacks;

typedef struct
{
	const char* name;
	const char* displayName;
	int type;
	V defaultValue;
	const char** dropNames;
	V* dropValues;
} ItemKvDef;

typedef struct
{
	const char* name;
	const char* trueInput;
	const char* falseInput;
	const char* trueArg;
	const char* falseArg;
} InputDef;

typedef struct
{
	const char* name;
	const char* trueOutput;
	const char* falseOutput;
} OutputDef;

typedef struct
{
	char type;

	void* data;

	const char* name;
	const char* group;

	const char* exportScript;

	InputDef* inputs;
	OutputDef* outputs;

	ItemKvDef* kvs;
	char** staticKvs;

  char snapMode;
  char transparent;
	char deleteIntersectingVoxels;
	char genMissingVoxels;

	ItemCallbacks* callbacks;
} ItemDefinition;

typedef struct
{
	ItemKvDef* def;
	V value;
} ItemKv;


typedef struct
{
	OutputDef* def;
	int entity;
	int antline;
	InputDef* input;
	char inverted;
} ItemOutput;

struct Item
{
	int index;
	int loadIndex;
	int id;
	vec3 pos;
	vec3 dir;
	vec4 quat;
	mat4 transform;
	mat4 invTransform;
  char snapDir;
  const char* ioEnt;

	ItemKv* kv;
	ItemOutput* outputs;

	ItemDefinition* def;

	void* data;
};

typedef struct
{
	const char* name;
	int size;
  int startInd;
} ItemGroup;

void loadItemDefinitions();

void drawItems();

void updateItemTransform(Item* item);
void updateItemTransformRot(Item* item);
Item* getIntersectingItem(vec3 pos, Item** ignore);
Item** getIntersectingItems(vec3 pos, Item** ignore);
Item* getItem(int i);

ItemDefinition* getItemDefinitions();

Item* getItemList();
Item* addItem(int id, ivec3 position);
Item* addItemFromDef(ItemDefinition* def, ivec3 position);
void removeItem(Item* item);
char isItemValid(Item* item);
