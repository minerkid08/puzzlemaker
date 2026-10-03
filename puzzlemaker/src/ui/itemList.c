#include "dynList.h"
#include <stdbool.h>
#include <stdio.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern Item* selectedItem;

static char buf[64];
void itemListRender()
{
	igBegin("Item List", 0, 0);

	Item* items = getItemList();
	int len = dynList_size(items);

	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		igPushID_Int(i);
		snprintf(buf, 64, "%s%d", item->def->name, item->index);
		char selected = (item == selectedItem);
		ImVec2 zero;
		zero.x = 0;
		zero.y = 0;
		if(igSelectable_Bool(buf, selected, 0, zero))
			selectedItem = item;
		igPopID();
	}

	igEnd();
}
