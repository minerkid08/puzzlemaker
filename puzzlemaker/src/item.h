#pragma once

#include "renderer/mesh.h"
#include <cglm/cglm.h>

#define TYPE_BOOL 1
#define TYPE_INT 2

#define ENTITY_TYPE_INSTANCE 1
#define ENTITY_TYPE_ENTITY 2

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
} ItemKvDef;

typedef struct
{
	const char* name;
	const char* trueInput;
	const char* falseInput;
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

	const char* name;
	const char* modelName;
	const char* textureName;
	const char* instanceName;
	ItemKvDef* kvs;
  char** staticKvs;

	InputDef* inputs;
	OutputDef* outputs;

	Mesh mesh;
	unsigned int texture;

	vec4 bound1;
	vec4 bound2;
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
	Item* entity;
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
};

void initItems();

void drawItems();

void updateItemTransform(Item* item);
void updateSelectedItem();
Item* findSelectedItem(vec3 pos, vec3 dir, float len);
Item* getSelectedItem();
void setSelectedItem(Item* item);
void startMove();
void endMove();

void addItemDef(ItemDefinition* def);
ItemDefinition* getItemDefinitions();
Item** getItems();

Item* addItem(int id);
void removeItem(Item* item);
