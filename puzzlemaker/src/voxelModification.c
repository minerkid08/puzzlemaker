#include "voxelModification.h"

#include "utils.h"
#include "voxel.h"
#include <string.h>

void voxelPush()
{
	if (currentVoxel == 0)
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
