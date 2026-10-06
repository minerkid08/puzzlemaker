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
	igSeparatorText("Antline");
	int segmentCount = dynList_size(selection.antline->segments);
	if(!selection.antline->hasCheck && segmentCount == 0)
		igText("warning: antline has no visible component");
	if (igButton("remove", zero))
		removeAntline(selection.antline);
	igCheckbox("check enable", (bool*)&selection.antline->hasCheck);
	igBeginDisabled(!selection.antline->hasCheck);
	if (igDragFloat3("position", selection.antline->baseSegment.pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
		antlineUpdateTransform(&selection.antline->baseSegment);
	if (igDragFloat3("rotation", selection.antline->baseSegment.rot, 0.01f, 0.0f, 0.0f, "%.3f", 0))
		antlineUpdateTransformRot(&selection.antline->baseSegment);
	igEndDisabled();

	igSeparatorText("segments");

	if (igButton("add", zero))
	{
		dynList_resize((void**)&selection.antline->segments, segmentCount + 1);
		memset(&selection.antline->segments[segmentCount], 0, sizeof(AntlineSegment));
		antlineUpdateTransformRot(&selection.antline->segments[segmentCount]);
		selection.antlineSeg = &selection.antline->segments[segmentCount];
		selection.antlineSeg->len = 1;
		segmentCount++;
	}

	for (int i = 0; i < segmentCount; i++)
	{
		int flags =
			ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		AntlineSegment* segment = &selection.antline->segments[i];
		if (segment == selection.antlineSeg)
			flags |= ImGuiTreeNodeFlags_Selected;
		char open = igTreeNodeEx_Ptr(segment, flags, "segment %d", i);

		if (igIsItemClicked(0))
			selection.antlineSeg = segment;

		if (open)
		{
			AntlineSegment** segList = &selection.antline->segments;
			if (igButton("remove", zero))
			{
				for (long long j = i; j < segmentCount - 1; j++)
					memcpy(&(*segList)[j], &(*segList)[j + 1], sizeof(AntlineSegment));
				segmentCount--;
				dynList_resize((void**)segList, segmentCount);
			}
			if (igDragFloat3("position", segment->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
				antlineUpdateTransform(segment);
			if (igDragFloat3("rotation", segment->rot, 0.01f, 0.0f, 0.0f, "%.3f", 0))
				antlineUpdateTransformRot(segment);
			igDragInt("length", &segment->len, 1, 1, 9999, "%d", 0);
			igTreePop();
		}
	}
}
