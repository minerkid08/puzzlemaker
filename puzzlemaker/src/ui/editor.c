#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern ImVec2 zero;

void renderEditorPanel()
{
	igBegin("Item Editor", 0, 0);
	if (igButton("crash :)", zero))
	{
		int* ptr = 0;
		*ptr = 7;
	}
	itemPanelStub();
	igSameLine(0, -1);
	antlinePanelStub();

	if (selection.type == SELECTION_ITEM)
		itemPanelRender();
	else if (selection.type == SELECTION_ANTLINE)
		antlinePanelRender();

	igEnd();
}
