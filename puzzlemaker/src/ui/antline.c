#include "antline/antline.h"
#include "dynList.h"
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern ImVec2 zero;

void antlinePanelStub()
{
	if (igButton("add antline", zero))
	{
		selection.type = SELECTION_ANTLINE;
		selection.antline = addAntline();
		selection.antlineSeg = &selection.antline->baseSegment;
	}
}

void antlinePanelRender()
{
	igCheckbox("check enable", (bool*)&selection.antline->hasCheck);
	if (igDragFloat3("position", selection.antline->baseSegment.pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
		antlineUpdateTransform(&selection.antline->baseSegment);
	if (igDragFloat3("rotation", selection.antline->baseSegment.rot, 0.01f, 0.0f, 0.0f, "%.3f", 0))
		antlineUpdateTransformRot(&selection.antline->baseSegment);

	igSeparatorText("segments");

	int segmentCount = dynList_size(selection.antline->segments);
	if (igButton("add", zero))
	{
		dynList_resize((void**)&selection.antline->segments, segmentCount + 1);
		memset(&selection.antline->segments[segmentCount], 0, sizeof(AntlineSegment));
		antlineUpdateTransformRot(&selection.antline->segments[segmentCount]);
		selection.antlineSeg = &selection.antline->segments[segmentCount];
		segmentCount++;
	}

	for (int i = 0; i < segmentCount; i++)
	{
		AntlineSegment* segment = &selection.antline->segments[i];
		if (igTreeNode_Ptr(segment, "segment %d", i))
		{
			if (igDragFloat3("position", segment->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
				antlineUpdateTransform(segment);
			if (igDragFloat3("rotation", segment->rot, 0.01f, 0.0f, 0.0f, "%.3f", 0))
				antlineUpdateTransformRot(segment);
			igDragInt("length", &segment->len, 1, 0, 9999, "%d", 0);
			igTreePop();
		}
	}
}
