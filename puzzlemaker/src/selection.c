#include "selection.h"
#include "camera.h"
#include "dynList.h"
#include "item/item.h"
#include "picker.h"
#include "raycast.h"
#include "ui/itemPanel.h"
#include "utils.h"

Selection selection;
extern Picker picker;

#define RAY_LEN 40

static char mode = 0;

char isSelecting()
{
	return mode;
}

void clearSelection()
{
	selection.type = SELECTION_NONE;
}

void beginSelection(vec3 mouseDir)
{
	Item** ignoredItems = getIntersectingItems(cameraPos, 0);

	int flags = RAYCAST_VOXEL | RAYCAST_ITEM | RAYCAST_ANTLINE;
	RaycastHit hit;
	if (raycast(cameraPos, mouseDir, RAY_LEN, flags, &hit, ignoredItems))
	{
		switch (hit.type)
		{
		case RAYCAST_VOXEL:
			selection.type = SELECTION_VOXEL;
			selection.voxel = hit.voxel;
			selection.voxelDir = hit.dir;
			selection.voxelPos[0] = hit.ipos[0];
			selection.voxelPos[1] = hit.ipos[1];
			selection.voxelPos[2] = hit.ipos[2];
			selection.voxel2Pos[0] = -1;
			selection.voxel2Pos[1] = -1;
			selection.voxel2Pos[2] = -1;
			mode = 1;
			picker.active = 0;
			break;
		case RAYCAST_ITEM:
			selection.voxel = 0;
			if (picker.active == PICKER_ITEM)
			{
				*picker.ptr = hit.item;
				picker.active = 0;
			}
			else
			{
				selection.type = SELECTION_ITEM;
				selection.item = hit.item;
				picker.active = 0;
			}
			break;
		case RAYCAST_ANTLINE:
			if (picker.active == PICKER_ANTLINE)
			{
				*picker.ant = hit.antline;
				picker.active = 0;
			}
			else
			{
				if (hit.antlineSeg)
				{
					selection.type = SELECTION_ANTLINE;
					selection.antline = hit.antline;
					selection.antlineSeg = hit.antlineSeg;
					picker.active = 0;
				}
				else
				{
					selection.type = SELECTION_ANTLINE;
					selection.antline = hit.antline;
					selection.antlineSeg = &selection.antline->baseSegment;
					picker.active = 0;
				}
			}
      break;
		}
	}
	else
	{
		selection.type = SELECTION_NONE;
		picker.active = 0;
		selection.voxel = 0;
	}
	dynList_free(ignoredItems);
}

void updateSelection(vec3 mouseDir)
{
	int flags = RAYCAST_VOXEL;
	RaycastHit hit;
	if (raycast(cameraPos, mouseDir, RAY_LEN, flags, &hit, 0))
	{
		int zmin = min(selection.voxelPos[2], hit.ipos[2]);
		int zmax = max(hit.ipos[2], selection.voxel2Pos[2]);
		int ymin = min(selection.voxelPos[1], hit.ipos[1]);
		int ymax = max(hit.ipos[1], selection.voxel2Pos[1]);
		int xmin = min(selection.voxelPos[0], hit.ipos[0]);
		int xmax = max(hit.ipos[0], selection.voxel2Pos[0]);

		selection.voxelPos[0] = xmin;
		selection.voxelPos[1] = ymin;
		selection.voxelPos[2] = zmin;

		selection.voxel2Pos[0] = xmax;
		selection.voxel2Pos[1] = ymax;
		selection.voxel2Pos[2] = zmax;
	}
	else
		selection.voxel = 0;
}

void endSelection()
{
	mode = 0;
}
