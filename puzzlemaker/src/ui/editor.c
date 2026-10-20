#include "camera.h"
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

	igDragFloat3("cam", cameraPos, 0.1, -9999, 9999, "%.2f", 0);
	if(igDragFloat3("rot", cameraRot, 0.01, -9999, 9999, "%.2f", 0))
		cameraRot[2] = 0;

	itemPanelStub();
	igSameLine(0, -1);
	antlinePanelStub();

	if (selection.type == SELECTION_ITEM)
		itemPanelRender();
	else if (selection.type == SELECTION_ANTLINE)
		antlinePanelRender();

	igEnd();
}
