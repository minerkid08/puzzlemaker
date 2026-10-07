#include "antline/antline.h"
#include "dynList.h"
#include <stdbool.h>
#include <stdio.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern Antline* antlines;

static char buf[64];
void itemListRender()
{
	ImVec2 buttonSize;
	buttonSize.x = 15;
	buttonSize.y = igGetTextLineHeight();

	igBegin("Item List", 0, 0);

	int flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;
	if (igTreeNodeEx_Str("items", flags))
	{
		Item* items = getItemList();
		int len = dynList_size(items);

		for (int i = 0; i < len; i++)
		{
			Item* item = &items[i];
			if (!isItemValid(item))
				continue;
			igPushID_Int(i);
			snprintf(buf, 64, "%s: %d", item->def->name, item->index);
			char selected = 0;
			if (selection.type == SELECTION_ITEM)
				selected = (item == selection.item);
			ImVec2 zero;
			zero.x = 0;
			zero.y = 0;
			if (igSelectable_Bool(buf, selected, ImGuiSelectableFlags_AllowOverlap, zero))
			{
				selection.type = SELECTION_ITEM;
				selection.item = item;
			}
			igSameLine(0, -1);
			char pressed;
			if (item->hidden)
				pressed = igButton("v", buttonSize);
			else
				pressed = igButton("h", buttonSize);
			if (pressed)
				item->hidden = !item->hidden;
			igPopID();
		}
		igTreePop();
	}

	if (igTreeNodeEx_Str("antlines", flags))
	{
		int len = dynList_size(antlines);

		for (int i = 0; i < len; i++)
		{
			Antline* antline = &antlines[i];
			if (!isAntlineValid(antline))
				continue;
			igPushID_Int(i);
			snprintf(buf, 64, "antline %d", antline->id);
			char selected = 0;
			if (selection.type == SELECTION_ANTLINE)
				selected = (antline == selection.antline);
			ImVec2 zero;
			zero.x = 0;
			zero.y = 0;
			if (igSelectable_Bool(buf, selected, 0, zero))
			{
				selection.type = SELECTION_ANTLINE;
				selection.antline = antline;
				selection.antlineSeg = &antline->baseSegment;
			}
			igPopID();
		}
		igTreePop();
	}
	igEnd();
}
