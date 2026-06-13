#include "camera.h"
#include "picker.h"
#include "raycast.h"
#include "utils.h"
#include "ui/itemPanel.h"
#include "voxel/voxel.h"

extern Picker picker;

#define RAY_LEN 20

static char mode = 0;

char isSelecting()
{
  return mode;
}

void beginSelection(vec3 mouseDir)
{
	int flags = RAYCAST_VOXEL | RAYCAST_ITEM;
	RaycastHit hit;
	if (raycast(cameraPos, mouseDir, RAY_LEN, flags, &hit))
	{
		if (hit.type == RAYCAST_VOXEL)
		{
			clearSelectedItem();
			currentVoxel = hit.voxel;
			currentDir = hit.dir;
			currentVoxelPos[0] = hit.pos[0];
			currentVoxelPos[1] = hit.pos[1];
			currentVoxelPos[2] = hit.pos[2];
			currentVoxel2Pos[0] = -1;
			currentVoxel2Pos[1] = -1;
			currentVoxel2Pos[2] = -1;
      mode = 1;
		}
		else
		{
			currentVoxel = 0;
			if (picker.active)
			{
				*picker.ptr = hit.item;
				picker.active = 0;
			}
			else
				setSelectedItem(hit.item);
		}
	}
	else
		currentVoxel = 0;
}

void updateSelection(vec3 mouseDir)
{
	int flags = RAYCAST_VOXEL;
	RaycastHit hit;
	if (raycast(cameraPos, mouseDir, RAY_LEN, flags, &hit))
	{
		int zmin = min(currentVoxelPos[2], hit.pos[2]);
		int zmax = max(hit.pos[2], currentVoxel2Pos[2]);
		int ymin = min(currentVoxelPos[1], hit.pos[1]);
		int ymax = max(hit.pos[1], currentVoxel2Pos[1]);
		int xmin = min(currentVoxelPos[0], hit.pos[0]);
		int xmax = max(hit.pos[0], currentVoxel2Pos[0]);

		currentVoxelPos[0] = xmin;
		currentVoxelPos[1] = ymin;
		currentVoxelPos[2] = zmin;

		currentVoxel2Pos[0] = xmax;
		currentVoxel2Pos[1] = ymax;
		currentVoxel2Pos[2] = zmax;
	}
	else
		currentVoxel = 0;
}

void endSelection()
{
  mode = 0;
}
