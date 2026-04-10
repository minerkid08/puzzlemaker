#pragma once

#include <cglm/cglm.h>

typedef union
{
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
	const char* falseOutput;
} InputDef;

typedef struct
{
	const char* name;
	const char* trueOutput;
	const char* falseOutput;
	char inverted;
} OutputDef;

typedef struct
{
	const char* name;
	const char* modelName;
	const char* textureName;
	ItemKvDef* kvs;
	
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
