#include "cglm/quat.h"
#include "dynList.h"
#include "item/item.h"
#include "utils.h"
#include <string.h>

Item* itemList;

static __attribute__((constructor)) void init()
{
	itemList = dynList_new(0, sizeof(Item));
	dynList_reserve((void**)&itemList, 32);
	for (int i = 0; i < 32; i++)
	{
		itemList[i].index = -1;
		itemList[i].id = -1;
		itemList[i].def = 0;
	}
}

Item* getItemList()
{
	return itemList;
}

void removeItem(Item* item)
{
	int itemCount = dynList_size(itemList);
	for (int i = 0; i < itemCount; i++)
	{
		Item* item2 = &itemList[i];
		if (!isItemValid(item2))
			continue;
		ItemOutput* outputs = item2->outputs;
		int outputCount = dynList_size(outputs);
		for (int j = 0; j < outputCount; j++)
		{
			ItemOutput* output = &outputs[j];
			if (output->entity == item->index)
			{
				output->entity = -1;
				output->input = 0;
			}
		}
		ItemKv* kvs = item2->kv;
		int kvCount = dynList_size(kvs);
		for (int j = 0; j < kvCount; j++)
		{
			ItemKv* kv = &kvs[j];
			int type = kv->def->type;
			type &= ~(TYPE_INSTANCE);
			if (type != TYPE_PICKER)
				continue;
			if (kv->value.i == item->index)
				kv->value.i = -1;
		}
	}

	ItemDefinition* def = item->def;
	int l = dynList_size(item->kv);
	for (int i = 0; i < l; i++)
	{
		int type = def->kvs[i].type;
		type &= ~(TYPE_INSTANCE);
		if (type == TYPE_STRING)
			free(item->kv[i].value.s);
	}

	dynList_free(item->kv);
	dynList_free(item->outputs);
	free(item->data);

	item->id = -1;
	item->index = -1;
	item->def = 0;
	item->data = 0;
}

Item* getItem(int i)
{
	return &itemList[i];
}

Item* addItemFromDef(ItemDefinition* def, ivec3 position)
{
	ItemDefinition* definitions = getItemDefinitions();
	for (int i = 0; i < dynList_size(definitions); i++)
	{
		if (strcmp(definitions[i].name, def->name) == 0)
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
		if (!isItemValid(item))
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

	item->snapDir = DIR_NONE;

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

	vec4 quat;
	glm_quat_identity(quat);
	memcpy(item->quat, quat, sizeof(vec4));

	updateItemTransform(item);

	ItemDefinition* def = &getItemDefinitions()[defId];
	item->def = def;

	item->outputs = dynList_new(0, sizeof(ItemOutput));
	int l = dynList_size(def->kvs);
	item->kv = dynList_new(l, sizeof(ItemKv));
	for (int i = 0; i < l; i++)
	{
		ItemKvDef* kvDef = &def->kvs[i];
		ItemKv* kv = &item->kv[i];
		item->kv[i].def = &def->kvs[i];
		int type = def->kvs[i].type;
		type &= ~(TYPE_INSTANCE);
		if (type == TYPE_STRING)
		{
			item->kv[i].value.s = malloc(256);
			strncpy(item->kv[i].value.s, def->kvs[i].defaultValue.s, 256);
		}
		else
			item->kv[i].value = def->kvs[i].defaultValue;
	}

	item->def->callbacks->init(item);

	return item;
}
