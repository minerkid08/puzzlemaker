#include "antline/antline.h"
#include "camera.h"
#include "dynList.h"
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"
#include "grab.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern ImVec2 zero;

void antlinePanelStub()
{
	if (igButton("Add antline", zero))
	{
		selection.type = SELECTION_ANTLINE;
		selection.antline = addAntline();
		selection.antlineSeg = &selection.antline->baseSegment;
		updateGrab(cameraPos, forward);
	}
}

void antlinePanelRender()
{
	igSeparatorText("Antline");
	int segmentCount = dynList_size(selection.antline->segments);
	if (!selection.antline->hasCheck && segmentCount == 0)
		igText("Warning: antline has no visible component");

	if (igButton("Remove", zero))
		removeAntline(selection.antline);

	igSameLine(0, -1);

	igBeginDisabled(!selection.antline->hasCheck);
	if (igButton("Select", zero))
		selection.antlineSeg = &selection.antline->baseSegment;
	igEndDisabled();
	igCheckbox("Check Enable", (bool*)&selection.antline->hasCheck);

	igBeginDisabled(!selection.antline->hasCheck);

	if (igDragFloat3("Position", selection.antline->baseSegment.pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
		antlineUpdateTransform(&selection.antline->baseSegment);

	if (igDragFloat3("Rotation", selection.antline->baseSegment.rot, 0.01f, 0.0f, 0.0f, "%.3f", 0))
		antlineUpdateTransformRot(&selection.antline->baseSegment);

	igEndDisabled();

	igSeparatorText("Segments");

	if (igButton("Add", zero))
	{
		dynList_resize((void**)&selection.antline->segments, segmentCount + 1);
		memset(&selection.antline->segments[segmentCount], 0, sizeof(AntlineSegment));
		antlineUpdateTransformRot(&selection.antline->segments[segmentCount]);
		selection.antlineSeg = &selection.antline->segments[segmentCount];
		selection.antlineSeg->len = 1;
		selection.antlineSeg->cornerMode = CORNER_MODE_AUTO;
		updateGrab(cameraPos, forward);
		segmentCount++;
	}

	for (int i = 0; i < segmentCount; i++)
	{
		int flags =
			ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		AntlineSegment* segment = &selection.antline->segments[i];
		if (segment == selection.antlineSeg)
			flags |= ImGuiTreeNodeFlags_Selected;
		char open = igTreeNodeEx_Ptr(segment, flags, "Segment %d", i);

		if (igIsItemClicked(0))
			selection.antlineSeg = segment;

		if (open)
		{
			AntlineSegment** segList = &selection.antline->segments;
			if (igButton("Remove", zero))
			{
				for (long long j = i; j < segmentCount - 1; j++)
					memcpy(&(*segList)[j], &(*segList)[j + 1], sizeof(AntlineSegment));
				segmentCount--;
				dynList_resize((void**)segList, segmentCount);
				if (i == segmentCount)
					selection.antlineSeg = &selection.antline->segments[segmentCount - 1];
			}
			if (igDragFloat3("Position", segment->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
				antlineUpdateTransform(segment);
			if (igDragFloat3("Rotation", segment->rot, 0.01f, 0.0f, 0.0f, "%.3f", 0))
				antlineUpdateTransformRot(segment);
			igDragInt("Length", &segment->len, 1, 1, 9999, "%d", 0);
			igCombo_Str("Corner Mode", &segment->cornerMode, "Auto\0Enabled\0Disabled\0", -1);
			igSetItemTooltip("Sets weather the segment will start with a corner.\n"
							 "Auto: Will be determined by if the start of the segment intersects with another segment\n"
							 "Enabled: The segment will always start with a corner\n"
							 "Disabled: The segment will not start with a corner");
			igTreePop();
		}
	}
}
