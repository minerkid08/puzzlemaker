#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "item/item.h"
#include "jsonUtils.h"
#include <GLFW/glfw3.h>
#include <string.h>

extern GLFWwindow* window;
extern ItemGroup* groups;

void copyItem(Item* item)
{
	ItemDefinition* def = item->def;
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "id", def->id);
	cJSON_AddStringToObject(json, "group", def->group);

	cJSON* kvList = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "kv", kvList);
	int l = dynList_size(item->kv);
	for (int i = 0; i < l; i++)
	{
		ItemKv* kv = &item->kv[i];
		cJSON* item;
		int type = kv->def->type;
		type &= ~(TYPE_INSTANCE);
		if (type == TYPE_INT)
			item = cJSON_CreateNumber(kv->value.i);
		if (type == TYPE_FLOAT)
			item = cJSON_CreateNumber(kv->value.f);
		if (type == TYPE_BOOL)
			item = cJSON_CreateBool(kv->value.b);
		if (type == TYPE_STRING)
			item = cJSON_CreateString(kv->value.s);
		if (type == TYPE_BITFIELD)
		{
			int value = 0;
			int len = dynList_size(kv->def->dropNames);
			for (int j = 0; j < len; j++)
			{
				if (kv->value.s[j])
					value |= 1 << kv->def->dropValues[j].i;
			}
			item = cJSON_CreateNumber(value);
		}
		if (type == TYPE_PICKER)
			item = cJSON_CreateNumber(kv->value.i);
		if (type & TYPE_DROPDOWN)
			item = cJSON_CreateNumber(kv->value.i);
		cJSON_AddItemToObject(kvList, kv->def->name, item);
	}

	def->callbacks->save(item, json);

	char* data = cJSON_PrintUnformatted(json);
	glfwSetClipboardString(window, data);
	free(data);
	cJSON_Delete(json);
}

Item* item;

static void pasteInt()
{
	const char* data = glfwGetClipboardString(window);
	cJSON* json = cJSON_Parse(data);
	if (json == 0)
	{
		const char* err = cJSON_GetErrorPtr();
		jsonParseError(data, err, "paste");
		openErrorPopup();
		return;
	}

	const char* defId = jsonGetStr(json, "id");
	const char* defGroup = jsonGetStr(json, "group");
	ItemDefinition* defs = getItemDefinitions();
	Item* item = 0;

	for (int i = 0; i < dynList_size(groups); i++)
	{
		ItemGroup* group = &groups[i];
		if (strcmp(group->name, defGroup))
			continue;
		for (int j = 0; j < group->size; j++)
		{
			if (strcmp(defs[j + group->startInd].id, defId) == 0)
			{
				item = addItem(j + group->startInd, 0);
				break;
			}
		}
		if (item)
			break;
	}
	if (item == 0)
		errorf("invalid item name\n");

	cJSON* kvJson = jsonGetObject(json, "kv");
	int len = dynList_size(item->def->kvs);
	for (int i = 0; i < len; i++)
	{
		ItemKv* kv = &item->kv[i];
		ItemKvDef* def = kv->def;

		int type = def->type;
		type &= ~(TYPE_INSTANCE);
		if (cJSON_GetObjectItem(kvJson, def->name))
		{
			if (type == TYPE_INT)
				kv->value.i = jsonGetInt(kvJson, def->name);
			if (type == TYPE_FLOAT)
				kv->value.f = jsonGetFloat(kvJson, def->name);
			if (type == TYPE_BOOL)
				kv->value.b = jsonGetBool(kvJson, def->name);
			if (type == TYPE_PICKER)
			{
				kv->value.i = jsonGetInt(kvJson, def->name);
				Item* itemList = getItemList();
				if (dynList_size(itemList) <= kv->value.i)
					kv->value.i = -1;
				else if (!isItemValid(getItem(kv->value.i)))
					kv->value.i = -1;
			}
			if (type == TYPE_STRING)
			{
				const char* str = jsonGetStr(kvJson, def->name);
				strncpy(kv->value.s, str, 256);
			}
			if (type == TYPE_BITFIELD)
			{
				int value = jsonGetInt(kvJson, def->name);
				int len = dynList_size(kv->def->dropNames);
				for (int j = 0; j < len; j++)
				{
					if (value & (1 << kv->def->dropValues[j].i))
						kv->value.s[j] = 1;
					else
						kv->value.s[j] = 0;
				}
			}
			if (type & TYPE_DROPDOWN)
			{
				int v = jsonGetInt(kvJson, def->name);
				int l = dynList_size(def->dropNames);

				if (v < 0 || v >= l)
					v = 0;
				kv->value.i = v;
			}
		}
	}
	popStackFrame();
	item->def->callbacks->load(item, json);
}

Item* pasteItem()
{
	if (initErrorStack("paste", pasteInt))
		openErrorPopup();
	return item;
}
