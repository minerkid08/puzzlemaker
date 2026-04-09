#include "item.h"

#include "renderer/texture.h"

#include <dynList.h>

ItemDefinition* definitions;
Item* items;

Item* selectedItem;

char moving = 0;

void initItems()
{
	definitions = dynList_new(0, sizeof(ItemDefinition));
	items = dynList_new(0, sizeof(Item));
}

void drawItems();

void updateSelectedItem();
Item* findSelectedItem(vec3 pos, vec3 dir, float len);
Item* getSelectedItem();
void setSelectedItem(Item* item);
void startMove();
void endMove();

void addItemDef(ItemDefinition* def)
{
	int len = dynList_size(definitions);
	dynList_resize((void**)&definitions, len + 1);
	definitions[len] = *def;
  definitions[len].texture = loadTexture(def->material);
}

ItemDefinition* getItemDefinitions()
{
  return definitions;
}

Item* addItem(int id)
{
	int len = dynList_size(items);
	dynList_resize((void**)&items, len + 1);
	Item* item = &items[len];
	item->id = id;

	item->dir[0] = 0;
	item->dir[1] = 0;
	item->dir[2] = 0;

	item->pos[0] = 0;
	item->pos[1] = 0;
	item->pos[2] = 0;
	return item;
}

void removeItem(int id);
