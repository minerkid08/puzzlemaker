#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item.h"

#include "cimgui.h"
#include "dynList.h"

const char** itemNames;

int defCount;

void initItemPanel()
{
	ItemDefinition* defs = getItemDefinitions();

	defCount = dynList_size(defs);

	itemNames = dynList_new(defCount, sizeof(const char*));

	for (int i = 0; i < defCount; i++)
		itemNames[i] = defs[i].name;
}

void itemPanelRender()
{
	igBegin("items", 0, 0);
	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;

	if (igButton("add", zero))
		igOpenPopup("pick");

	if (igBeginPopup("pick", 0))
	{
		for(int i = 0; i < defCount; i++)
		{
			if(igSelectable(itemNames[i], 0, 0, zero))
				printf("%s\n", itemNames[i]);
		}
		igEndPopup();
	}
	igEnd();
}
