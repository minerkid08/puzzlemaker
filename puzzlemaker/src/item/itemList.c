#include "dynList.h"
#include "entityItem.h"
#include "item/item.h"
#include "item/panel.h"
#include "volumeItem.h"
#include <string.h>

Item* itemList;

__attribute__((constructor)) static void init()
{
	itemList = dynList_new(0, sizeof(Item));
}

Item* getItemList()
{
	return itemList;
}

void removeItem(Item* item)
{
	dynList_free(item->kv);
	dynList_free(item->outputs);

	item->id = -1;
	item->index = -1;
}

Item* getItem(int i)
{
	return &itemList[i];
}

Item* addItemFromDef(ItemDefinition* def, ivec3 position)
{
  ItemDefinition* definitions = getItemDefinitions();
  for(int i = 0; i < dynList_size(definitions); i++)
  {
    if(strcmp(definitions[i].name, def->name) == 0)
      return addItem(i, position);
  }
  return 0;
}

Item* addItem(int defId, ivec3 position)
{
	int len = dynList_size(itemList);
	int index = -1;
	for (int i = 0; i < len; i++)
	{
		Item* item = &itemList[i];
		if (item->index == -1)
		{
			index = i;
			break;
		}
	}
	if (index == -1)
	{
		dynList_resize((void**)&itemList, len + 1);
		index = len;
	}
	Item* item = &itemList[index];
	item->index = index;
	item->id = defId;

	item->dir[0] = 0;
	item->dir[1] = 0;
	item->dir[2] = 0;

	if (position == 0)
	{
		item->pos[0] = 0;
		item->pos[1] = 0;
		item->pos[2] = 0;
	}
	else
	{
		item->pos[0] = position[0];
		item->pos[1] = position[1];
		item->pos[2] = position[2];
	}

	updateItemTransform(item);

	ItemDefinition* def = &getItemDefinitions()[defId];
	item->def = def;

	item->outputs = dynList_new(0, sizeof(ItemOutput));
	int l = dynList_size(def->kvs);
	item->kv = dynList_new(l, sizeof(ItemKv));
	for (int i = 0; i < l; i++)
	{
		item->kv[i].def = &def->kvs[i];
		item->kv[i].value = def->kvs[i].defaultValue;
	}

	if (item->def->type == ITEM_TYPE_ENTITY)
		entityItemInit(item);
	if (item->def->type == ITEM_TYPE_PANEL)
		panelItemInit(item);
	if (item->def->type == ITEM_TYPE_VOLUME)
		volumeItemInit(item);

	return item;
}
