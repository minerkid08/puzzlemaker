#pragma once

#include "cjson.h"
#include "packages/package.h"
#include "renderer/mesh.h"
#include <cglm/cglm.h>

#define TYPE_BOOL 1
#define TYPE_INT 2
#define TYPE_STRING 3
#define TYPE_FLOAT 4
#define TYPE_PICKER 5
#define TYPE_BITFIELD 6
#define TYPE_VEC3 7
#define TYPE_INSTANCE 64
#define TYPE_DROPDOWN 128

#define GIZMO_NONE 0
#define GIZMO_POS 1
#define GIZMO_POS_OFFSET 2
#define GIZMO_ANGLE 3
#define GIZMO_ANGLE_OFFSET 4

#define ITEM_TYPE_ENTITY 1
#define ITEM_TYPE_PANEL 2
#define ITEM_TYPE_VOLUME 3
#define ITEM_TYPE_OVERLAY 4

#define ITEM_FLAGS_SNAP_MODE ((1 << 0) | (1 << 1))
#define ITEM_FLAGS_ROT_RESTRICT ((1 << 2) | (1 << 3) | (1 << 4))
#define ITEM_FLAGS_TRANSPARENT (1 << 5)
#define ITEM_FLAGS_OPAQUE (1 << 6)
#define ITEM_FLAGS_REQUIRES_PAINT (1 << 7)
#define ITEM_FLAGS_DELVOXELS (1 << 8)
#define ITEM_FLAGS_DELVOXELSEDITOR (1 << 9)
#define ITEM_FLAGS_GENVOXELS (1 << 10)

#define SNAP_CORNER 0
#define SNAP_CENTER 1
#define SNAP_MINI_CORNER 2
#define SNAP_MINI_CENTER 3

#define RESTRICT_X (1 << 2)
#define RESTRICT_Y (1 << 3)
#define RESTRICT_Z (1 << 4)

typedef struct Item Item;

typedef union {
	int i;
	float f;
	char b;
	char* s;
	float* v;
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
	void* data;
	Package* package;

	const char* id;
	const char* name;
	const char* group;

	const char* exportScript;

	InputDef* inputs;
	OutputDef* outputs;

	ItemKvDef* kvs;
	char** staticKvs;
	ItemCallbacks* callbacks;

	int type;
	int flags;
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
	char onlyPrimary;
} ItemOutput;

struct Item
{
	mat4 transform;
	vec4 quat;
	vec3 pos;
	vec3 dir;
	int index;
	int loadIndex;
	ItemKv* kv;
	ItemOutput* outputs;
	const char* ioEnt;
	ItemDefinition* def;
	void* data;
	int id;
	char highlighted;
	char snapDir;
	char hidden;
};

typedef struct
{
	const char* name;
	int size;
	int startInd;
} ItemGroup;

void loadItemDefinitionFile(Package* package, const char* filename);
void itemLoaderEnd();

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
Item* addItemFromDefName(const char* name, ivec3 position);
void removeItem(Item* item);
char isItemValid(Item* item);
char* itemProcessString(const char* str, ItemKv* kvs);

void copyItem(Item* item);
Item* pasteItem();
