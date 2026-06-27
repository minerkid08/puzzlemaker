#pragma once

#include "renderer/mesh.h"
#include <cglm/cglm.h>

#define TYPE_BOOL 1
#define TYPE_INT 2
#define TYPE_STRING 3
#define TYPE_DROPDOWN 128

#define ITEM_TYPE_ENTITY 1
#define ITEM_TYPE_PANEL 2
#define ITEM_TYPE_VOLUME 3

typedef union {
	int i;
	float f;
	char b;
	char* s;
} V;

typedef struct
{
	const char* name;
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

	InputDef* inputs;
	OutputDef* outputs;

	ItemKvDef* kvs;
	char** staticKvs;

	vec4 bound1;
	vec4 bound2;
	vec3 offset;
} ItemDefinition;

typedef struct
{
	ItemKvDef* def;
	V value;
} ItemKv;

typedef struct Item Item;

typedef struct
{
	OutputDef* def;
	int entity;
	InputDef* input;
	char inverted;
} ItemOutput;

struct Item
{
	int index;
	int id;
	vec3 pos;
	vec3 dir;
	mat4 transform;

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
Item* getIntersectingItem(vec3 pos);
Item* getItem(int i);

ItemDefinition* getItemDefinitions();

Item* getItemList();
Item* addItem(int id, ivec3 position);
Item* addItemFromDef(ItemDefinition* def, ivec3 position);
void removeItem(Item* item);
