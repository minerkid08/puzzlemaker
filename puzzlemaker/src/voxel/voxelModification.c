#include "voxelModification.h"

#include "selection.h"
#include "utils.h"

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

	int x = selection.voxelPos[axis3];
	if (action == ACTION_PULL)
	{
		if (dir > 0 && x >= MAP_SIZE - 1)
			return;

		if (dir < 0 && x <= 0)
			return;
	}
	if (action == ACTION_PUSH)
	{
		if (dir > 0 && x <= 1)
			return;

		if (dir < 0 && x >= MAP_SIZE - 2)
			return;
	}

	for (int y = selection.voxelPos[axis2]; y <= selection.voxel2Pos[axis2]; y++)
	{
		for (int x = selection.voxelPos[axis1]; x <= selection.voxel2Pos[axis1]; x++)
		{
			ivec3 pos;
			pos[axis1] = x;
			pos[axis2] = y;
			pos[axis3] = selection.voxelPos[axis3] - dir;

			if (action == ACTION_PUSH)
			{
				if (canPush(x, y, selection.voxelPos[axis3] - dir))
					getVoxelv(pos)->solid = 0;
			}
			else if (action == ACTION_PULL)
			{
				pos[axis3] += dir;
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
	if (action == ACTION_PULL)
	{
		selection.voxelPos[axis3] += dir;
		selection.voxel2Pos[axis3] += dir;
	}
	else
	{
		selection.voxelPos[axis3] -= dir;
		selection.voxel2Pos[axis3] -= dir;
	}
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
					getVoxel(x, y, z)->solid = 0;
				else if (action == ACTION_PULL)
					getVoxel(x, y, z)->solid = 1;
				else if (action == ACTION_PORT)
				{
					Voxel* v = getVoxel(x, y, z);
					if (v->solid)
					{
						v->portalability[0] ^= 1;
						v->portalability[1] ^= 1;
						v->portalability[2] ^= 1;
						v->portalability[3] ^= 1;
						v->portalability[4] ^= 1;
						v->portalability[5] ^= 1;
					}
					else
					{
						for (int i = 0; i < 6; i++)
						{
							ivec3* dir = &dirs[i];
							ivec3 pos = {x - (*dir)[0], y - (*dir)[1], z - (*dir)[2]};
							Voxel* v2 = getVoxelv(pos);
							if (v2->solid)
								v2->portalability[i] ^= 1;
						}
					}
				}
				else if (action == ACTION_SIZE)
				{
					Voxel* v = getVoxel(x, y, z);
					if (v->solid)
					{
						v->portalability[0] ^= 2;
						v->portalability[1] ^= 2;
						v->portalability[2] ^= 2;
						v->portalability[3] ^= 2;
						v->portalability[4] ^= 2;
						v->portalability[5] ^= 2;
					}
					else
					{
						for (int i = 0; i < 6; i++)
						{
							ivec3* dir = &dirs[i];
							ivec3 pos = {x - (*dir)[0], y - (*dir)[1], z - (*dir)[2]};
							Voxel* v2 = getVoxelv(pos);
							if (v2->solid)
								v2->portalability[i] ^= 2;
						}
					}
				}
			}
		}
	}
}

void voxelPush()
{
	if (isSelection2d())
		modify2dSelection(ACTION_PUSH);
	else
		modify3dSelection(ACTION_PUSH);
}

void voxelPull()
{
	if (isSelection2d())
		modify2dSelection(ACTION_PULL);
	else
		modify3dSelection(ACTION_PULL);
}

void voxelTogglePortal()
{
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
