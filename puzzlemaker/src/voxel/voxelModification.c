#include "voxelModification.h"

#include "utils.h"
#include "selection.h"
#include "voxel.h"
#include <string.h>

#define ACTION_PUSH 1
#define ACTION_PULL 2
#define ACTION_PORT 3
#define ACTION_SIZE 4

void modify2dSelection(int action)
{
	int axis1 = -1;
	int axis2 = -1;
	int axis3 = -1;
	int dir = (selection.voxelDir == DIR_POS_X || selection.voxelDir == DIR_POS_Y || selection.voxelDir == DIR_POS_Z);
	if (dir == 0)
		dir = -1;

	if (action == ACTION_PULL)
		dir *= -1;

	if (selection.voxelDir == DIR_POS_X || selection.voxelDir == DIR_NEG_X)
	{
		axis1 = 1;
		axis2 = 2;
		axis3 = 0;
	}

	if (selection.voxelDir == DIR_POS_Y || selection.voxelDir == DIR_NEG_Y)
	{
		axis1 = 2;
		axis2 = 0;
		axis3 = 1;
	}

	if (selection.voxelDir == DIR_POS_Z || selection.voxelDir == DIR_NEG_Z)
	{
		axis1 = 0;
		axis2 = 1;
		axis3 = 2;
	}

	int x = selection.voxelPos[axis3] - dir;
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

	for (int y = selection.voxelPos[axis2]; y <= selection.voxel2Pos[axis2]; y++)
	{
		for (int x = selection.voxelPos[axis1]; x <= selection.voxel2Pos[axis1]; x++)
		{
			ivec3 pos;
			pos[axis1] = x;
			pos[axis2] = y;
			pos[axis3] = selection.voxelPos[axis3];

			if (action == ACTION_PUSH)
			{
				if (canPush(x, y, selection.voxelPos[axis3]))
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
				v->portalability[selection.voxelDir] ^= 1;
			}
			else if (action == ACTION_SIZE)
			{
				Voxel* v = getVoxelv(pos);
				v->portalability[selection.voxelDir] ^= 2;
			}
		}
	}
	if (action == ACTION_PORT || action == ACTION_SIZE)
		return;
	selection.voxelPos[axis3] -= dir;
	selection.voxel2Pos[axis3] -= dir;
}

void modify3dSelection(int action)
{
	for (int z = selection.voxelPos[2]; z <= selection.voxel2Pos[2]; z++)
	{
		for (int y = selection.voxelPos[1]; y <= selection.voxel2Pos[1]; y++)
		{
			for (int x = selection.voxelPos[0]; x <= selection.voxel2Pos[0]; x++)
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
					v->portalability[0] ^= 1;
					v->portalability[1] ^= 1;
					v->portalability[2] ^= 1;
					v->portalability[3] ^= 1;
					v->portalability[4] ^= 1;
					v->portalability[5] ^= 1;
				}
				else if (action == ACTION_SIZE)
				{
					Voxel* v = getVoxel(x, y, z);
					v->portalability[0] ^= 2;
					v->portalability[1] ^= 2;
					v->portalability[2] ^= 2;
					v->portalability[3] ^= 2;
					v->portalability[4] ^= 2;
					v->portalability[5] ^= 2;
				}
			}
		}
	}
}

void voxelPush()
{
	if (selection.voxel == 0)
		return;

	if (selection.voxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_PUSH);
		else
			modify3dSelection(ACTION_PUSH);
		return;
	}

	if (selection.voxelPos[0] == 0 || selection.voxelPos[0] == MAP_SIZE - 1)
		return;
	if (selection.voxelPos[1] == 0 || selection.voxelPos[1] == MAP_SIZE - 1)
		return;
	if (selection.voxelPos[2] == 0 || selection.voxelPos[2] == MAP_SIZE - 1)
		return;

	selection.voxel->solid = 0;

	ivec3 dir;
	memcpy(dir, dirs[selection.voxelDir], sizeof(int) * 3);

	if (!inRange(selection.voxelPos[0] - dir[0], selection.voxelPos[1] - dir[1], selection.voxelPos[2] - dir[2]))
	{
		selection.voxel = 0;
		return;
	}

	selection.voxelPos[0] -= dir[0];
	selection.voxelPos[1] -= dir[1];
	selection.voxelPos[2] -= dir[2];

	selection.voxel = getVoxelv(selection.voxelPos);
}

void voxelPull()
{
	if (selection.voxel == 0)
		return;

	if (selection.voxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_PULL);
		else
			modify3dSelection(ACTION_PULL);
		return;
	}

	ivec3 newPos = {selection.voxelPos[0], selection.voxelPos[1], selection.voxelPos[2]};

	ivec3 dir;
	memcpy(dir, dirs[selection.voxelDir], sizeof(int) * 3);

	newPos[0] += dirs[selection.voxelDir][0];
	newPos[1] += dirs[selection.voxelDir][1];
	newPos[2] += dirs[selection.voxelDir][2];

	if (!inRange(newPos[0], newPos[1], newPos[2]))
	{
		selection.voxel = 0;
		return;
	}

	Voxel* v = getVoxelv(newPos);

	if (v->solid == 1)
	{
		selection.voxel = 0;
		return;
	}

	v->solid = 1;

	newPos[0] += dirs[selection.voxelDir][0];
	newPos[1] += dirs[selection.voxelDir][1];
	newPos[2] += dirs[selection.voxelDir][2];

	if (!inRange(newPos[0], newPos[1], newPos[2]))
	{
		selection.voxel = 0;
		return;
	}

	v = getVoxelv(newPos);

	if (v->solid == 1)
	{
		selection.voxel = 0;
		return;
	}

	selection.voxelPos[0] += dir[0];
	selection.voxelPos[1] += dir[1];
	selection.voxelPos[2] += dir[2];
	selection.voxel = getVoxelv(selection.voxelPos);
}

void voxelTogglePortal()
{
	if (selection.voxel == 0)
		return;

	if (selection.voxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_PORT);
		else
			modify3dSelection(ACTION_PORT);
		return;
	}

	selection.voxel->portalability[selection.voxelDir] ^= 1;
}

void voxelToggleSize()
{
	if (selection.voxel == 0)
		return;

	if (selection.voxel2Pos[0] >= 0)
	{
		if (isSelection2d())
			modify2dSelection(ACTION_SIZE);
		else
			modify3dSelection(ACTION_SIZE);
		return;
	}

	selection.voxel->portalability[selection.voxelDir] ^= 2;
}
