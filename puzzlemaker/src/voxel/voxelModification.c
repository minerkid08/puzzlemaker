#include "voxelModification.h"

#include "utils.h"
#include "voxel.h"
#include <string.h>

#define ACTION_PUSH 1
#define ACTION_PULL 2
#define ACTION_PORT 3

void modify2dSelection(int action)
{
	int axis1 = -1;
	int axis2 = -1;
	int axis3 = -1;
	int dir = (currentDir == DIR_POS_X || currentDir == DIR_POS_Y || currentDir == DIR_POS_Z);
	if (dir == 0)
		dir = -1;

	if (action == ACTION_PULL)
		dir *= -1;

	if (currentDir == DIR_POS_X || currentDir == DIR_NEG_X)
	{
		axis1 = 1;
		axis2 = 2;
		axis3 = 0;
	}

	if (currentDir == DIR_POS_Y || currentDir == DIR_NEG_Y)
	{
		axis1 = 2;
		axis2 = 0;
		axis3 = 1;
	}

	if (currentDir == DIR_POS_Z || currentDir == DIR_NEG_Z)
	{
		axis1 = 0;
		axis2 = 1;
		axis3 = 2;
	}

	int x = currentVoxelPos[axis3] - dir;
	if (action == ACTION_PUSH)
	{
		if (x < 0 || x >= MAP_SIZE)
			return;
	}
	if (action == ACTION_PULL)
	{
		if (x < 1 || x >= MAP_SIZE - 1)
			return;
	}

	for (int y = currentVoxelPos[axis2]; y <= currentVoxel2Pos[axis2]; y++)
	{
		for (int x = currentVoxelPos[axis1]; x <= currentVoxel2Pos[axis1]; x++)
		{
			ivec3 pos;
			pos[axis1] = x;
			pos[axis2] = y;
			pos[axis3] = currentVoxelPos[axis3];

			if (action == ACTION_PUSH)
			{
				if (canPush(x, y, currentVoxelPos[axis3]))
					getVoxelv(pos)->solid = 0;
			}
			else if (action == ACTION_PULL)
			{
				pos[axis3] -= dir;
				getVoxelv(pos)->solid = 1;
			}
			else if (action == ACTION_PORT)
			{
				Voxel* v = getVoxelv(pos);
				v->portalability[currentDir] = !v->portalability[currentDir];
			}
		}
	}
	if (action == ACTION_PORT)
		return;
	currentVoxelPos[axis3] -= dir;
	currentVoxel2Pos[axis3] -= dir;
}

void modify3dSelection(int action)
{
	for (int z = currentVoxelPos[2]; z <= currentVoxel2Pos[2]; z++)
	{
		for (int y = currentVoxelPos[1]; y <= currentVoxel2Pos[1]; y++)
		{
			for (int x = currentVoxelPos[0]; x <= currentVoxel2Pos[0]; x++)
			{
				if (action == ACTION_PUSH)
				{
					if (canPush(x, y, z))
						getVoxel(x, y, z)->solid = 0;
				}
				else if (action == ACTION_PULL)
					getVoxel(x, y, z)->solid = 1;
				else if (action == ACTION_PORT)
				{
					Voxel* v = getVoxel(x, y, z);
					v->portalability[0] = !v->portalability[0];
					v->portalability[1] = !v->portalability[1];
					v->portalability[2] = !v->portalability[2];
					v->portalability[3] = !v->portalability[3];
					v->portalability[4] = !v->portalability[4];
					v->portalability[5] = !v->portalability[5];
				}
			}
		}
	}
}

void voxelPush()
{
	if (currentVoxel == 0)
		return;

	if (currentVoxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_PUSH);
		else
			modify3dSelection(ACTION_PUSH);
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

	if (currentVoxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_PULL);
		else
			modify3dSelection(ACTION_PULL);
		return;
	}

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

	if (currentVoxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_PORT);
		else
			modify3dSelection(ACTION_PORT);
		return;
	}

	currentVoxel->portalability[currentDir] = !currentVoxel->portalability[currentDir];
}
