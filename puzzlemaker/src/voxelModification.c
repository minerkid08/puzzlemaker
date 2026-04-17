#include "voxelModification.h"

#include "utils.h"
#include "voxel.h"
#include <string.h>

void voxelPush()
{
	if (currentVoxel == 0)
		return;

	if (currentVoxel2Pos[0] >= 0)
	{
		int zmin = min(currentVoxelPos[2], currentVoxel2Pos[2]);
		int zmax = max(currentVoxelPos[2], currentVoxel2Pos[2]);
		int ymin = min(currentVoxelPos[1], currentVoxel2Pos[1]);
		int ymax = max(currentVoxelPos[1], currentVoxel2Pos[1]);
		int xmin = min(currentVoxelPos[0], currentVoxel2Pos[0]);
		int xmax = max(currentVoxelPos[0], currentVoxel2Pos[0]);

		for (int z = zmin; z <= zmax; z++)
		{
			for (int y = ymin; y <= ymax; y++)
			{
				for (int x = xmin; x <= xmax; x++)
				{
          getVoxel(x, y, z)->solid = 0;
				}
			}
		}
    currentVoxel2Pos[0] = -1;
    currentVoxel2Pos[1] = -1;
    currentVoxel2Pos[2] = -1;
    currentVoxel = 0;
    return;
	}

	if (currentVoxelPos[0] == 0 || currentVoxelPos[0] == MAP_SIZE - 1)
		return;
	if (currentVoxelPos[1] == 0 || currentVoxelPos[1] == MAP_SIZE - 1)
		return;
	if (currentVoxelPos[2] == 0 || currentVoxelPos[2] == MAP_SIZE - 1)
		return;

	currentVoxel->solid = 0;

	ivec3 dir;
	memcpy(dir, dirs[currentDir], sizeof(int) * 3);

	if (!inRange(currentVoxelPos[0] - dir[0], currentVoxelPos[1] - dir[1], currentVoxelPos[2] - dir[2]))
	{
		currentVoxel = 0;
		return;
	}

	currentVoxelPos[0] -= dir[0];
	currentVoxelPos[1] -= dir[1];
	currentVoxelPos[2] -= dir[2];

	currentVoxel = getVoxelv(currentVoxelPos);
}

void voxelPull()
{
	if (currentVoxel == 0)
		return;

	ivec3 newPos = {currentVoxelPos[0], currentVoxelPos[1], currentVoxelPos[2]};

	ivec3 dir;
	memcpy(dir, dirs[currentDir], sizeof(int) * 3);

	newPos[0] += dirs[currentDir][0];
	newPos[1] += dirs[currentDir][1];
	newPos[2] += dirs[currentDir][2];

	if (!inRange(newPos[0], newPos[1], newPos[2]))
	{
		currentVoxel = 0;
		return;
	}

	Voxel* v = getVoxelv(newPos);

	if (v->solid == 1)
	{
		currentVoxel = 0;
		return;
	}

	v->solid = 1;

	newPos[0] += dirs[currentDir][0];
	newPos[1] += dirs[currentDir][1];
	newPos[2] += dirs[currentDir][2];

	if (!inRange(newPos[0], newPos[1], newPos[2]))
	{
		currentVoxel = 0;
		return;
	}

	v = getVoxelv(newPos);

	if (v->solid == 1)
	{
		currentVoxel = 0;
		return;
	}

	currentVoxelPos[0] += dir[0];
	currentVoxelPos[1] += dir[1];
	currentVoxelPos[2] += dir[2];
	currentVoxel = getVoxelv(currentVoxelPos);
}

void voxelTogglePortal()
{
	if (currentVoxel == 0)
		return;

	currentVoxel->portalability[currentDir] = !currentVoxel->portalability[currentDir];
}
