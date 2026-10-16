#include "voxel/voxel.h"
#include "dynList.h"
#include "item/item.h"
#include "selection.h"
#include "utils.h"
#include "voxel/itemIntersection.h"
#include "voxel/voxelConfig.h"
#include <string.h>

Voxel voxels[MAP_SIZE * MAP_SIZE * MAP_SIZE];
ivec3 mapReigonStart;
ivec3 mapReigonEnd;

char isSelection2d()
{
	return selection.voxelDir != DIR_NONE;
}

char pointInRange(ivec3 point, ivec3 boundA, ivec3 boundB)
{
	char x = (boundA[0] <= point[0] && point[0] <= boundB[0]);
	char y = (boundA[1] <= point[1] && point[1] <= boundB[1]);
	char z = (boundA[2] <= point[2] && point[2] <= boundB[2]);

	return x && y && z;
}

void initVoxels()
{
	loadVoxelConfig();

	memset(voxels, 0, sizeof(voxels));
	for (int i = 0; i < VOXEL_COUNT; i++)
	{
		voxels[i].solid = 1;
	}

  int mapCenter = MAP_SIZE / 2;
  int horizStart = mapCenter - 2;
  int horizEnd = mapCenter + 3;
  int vertStart = mapCenter - 2;
  int vertEnd = mapCenter + 1;

  mapReigonStart[0] = horizStart - 1;
  mapReigonStart[1] = vertStart - 1;
  mapReigonStart[2] = horizStart - 1;

  mapReigonEnd[0] = horizEnd + 1;
  mapReigonEnd[1] = vertEnd + 1;
  mapReigonEnd[2] = horizEnd + 1;

	for (int z = horizStart; z < horizEnd; z++)
	{
		for (int y = vertStart; y < vertEnd; y++)
		{
			for (int x = horizStart; x < horizEnd; x++)
			{
				getVoxel(x, y, z)->solid = 0;
				getVoxel(x, y, z)->portalability[0] = 0;
				getVoxel(x, y, z)->portalability[1] = 0;
				getVoxel(x, y, z)->portalability[2] = 0;
				getVoxel(x, y, z)->portalability[3] = 0;
				getVoxel(x, y, z)->portalability[4] = 0;
				getVoxel(x, y, z)->portalability[5] = 0;
			}
		}
	}
	updateVoxelFaces();
}

Voxel* getVoxel(int x, int y, int z)
{
	assert(x >= 0 && x < MAP_SIZE);
	assert(y >= 0 && y < MAP_SIZE);
	assert(z >= 0 && z < MAP_SIZE);
	return &voxels[x + (y * MAP_SIZE) + (z * MAP_SIZE * MAP_SIZE)];
}

Voxel* getVoxelv(ivec3 pos)
{
	return getVoxel(pos[0], pos[1], pos[2]);
}

int getVoxelSide(vec3 start, ivec3 pos2, vec3 dir, vec3 pos)
{
	int x = pos2[0];
	int y = pos2[1];
	int z = pos2[2];

	vec3 invDir = {1.0f / dir[0], 1.0f / dir[1], 1.0f / dir[2]};
	vec3 blockMin = {x, y, z};
	vec3 blockMax = {x + 1, y + 1, z + 1};

	float tx1 = (blockMin[0] - start[0]) * invDir[0];
	float tx2 = (blockMax[0] - start[0]) * invDir[0];
	float tmin = min(tx1, tx2);
	float tmax = max(tx1, tx2);

	tx1 = (blockMin[1] - start[1]) * invDir[1];
	tx2 = (blockMax[1] - start[1]) * invDir[1];
	tmin = max(tmin, min(tx1, tx2));
	tmax = min(tmax, max(tx1, tx2));

	tx1 = (blockMin[2] - start[2]) * invDir[2];
	tx2 = (blockMax[2] - start[2]) * invDir[2];
	tmin = max(tmin, min(tx1, tx2));
	tmax = min(tmax, max(tx1, tx2));

	pos[0] = (tmin * dir[0]) + start[0];
	pos[1] = (tmin * dir[1]) + start[1];
	pos[2] = (tmin * dir[2]) + start[2];

	if (absf(pos[0] - x) < 0.0001f)
		return DIR_NEG_X;
	if (absf(pos[0] - x) > 0.9999f)
		return DIR_POS_X;
	if (absf(pos[1] - y) < 0.0001f)
		return DIR_NEG_Y;
	if (absf(pos[1] - y) > 0.9999f)
		return DIR_POS_Y;
	if (absf(pos[2] - z) < 0.0001f)
		return DIR_NEG_Z;
	if (absf(pos[2] - z) > 0.9999f)
		return DIR_POS_Z;
	return 0;
}

void updateVoxelFaces()
{
	for (int z = mapReigonStart[2]; z < mapReigonEnd[2]; z++)
	{
		for (int y = mapReigonStart[1]; y < mapReigonEnd[1]; y++)
		{
			for (int x = mapReigonStart[0]; x < mapReigonEnd[0]; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				voxel->faces = 0;
				if (!voxel->solid)
					continue;

				if (z + 1 < MAP_SIZE)
				{
					Voxel* v2 = getVoxel(x, y, z + 1);
					if (!v2->solid)
						voxel->faces |= (1 << DIR_POS_Z);
				}

				if (z - 1 >= 0)
				{
					Voxel* v2 = getVoxel(x, y, z - 1);
					if (!v2->solid)
						voxel->faces |= (1 << DIR_NEG_Z);
				}

				if (x + 1 < MAP_SIZE)
				{
					Voxel* v2 = getVoxel(x + 1, y, z);
					if (!v2->solid)
						voxel->faces |= (1 << DIR_POS_X);
				}

				if (x - 1 >= 0)
				{
					Voxel* v2 = getVoxel(x - 1, y, z);
					if (!v2->solid)
						voxel->faces |= (1 << DIR_NEG_X);
				}

				if (y + 1 < MAP_SIZE)
				{
					Voxel* v2 = getVoxel(x, y + 1, z);
					if (!v2->solid)
						voxel->faces |= (1 << DIR_POS_Y);
				}

				if (y - 1 >= 0)
				{
					Voxel* v2 = getVoxel(x, y - 1, z);
					if (!v2->solid)
						voxel->faces |= (1 << DIR_NEG_Y);
				}
			}
		}
	}
	updateCulledVoxels();
}

static void checkSubVoxel(Item* item, vec3 pos, float xoff, float yoff, float zoff, Voxel* voxel)
{
	vec3 newPos = {pos[0] + xoff / 2.0f, pos[1] + yoff / 2.0f, pos[2] + zoff / 2.0f};
	if (itemIntersectsVoxel(item, newPos, 0.5))
	{
		if (xoff == 0 && yoff == 0 && zoff == 0)
			voxel->subVoxels |= (1 << SUBVOXEL_xyz);
		if (xoff == 1 && yoff == 0 && zoff == 0)
			voxel->subVoxels |= (1 << SUBVOXEL_Xyz);
		if (xoff == 0 && yoff == 1 && zoff == 0)
			voxel->subVoxels |= (1 << SUBVOXEL_xYz);
		if (xoff == 1 && yoff == 1 && zoff == 0)
			voxel->subVoxels |= (1 << SUBVOXEL_XYz);
		if (xoff == 0 && yoff == 0 && zoff == 1)
			voxel->subVoxels |= (1 << SUBVOXEL_xyZ);
		if (xoff == 1 && yoff == 0 && zoff == 1)
			voxel->subVoxels |= (1 << SUBVOXEL_XyZ);
		if (xoff == 0 && yoff == 1 && zoff == 1)
			voxel->subVoxels |= (1 << SUBVOXEL_xYZ);
		if (xoff == 1 && yoff == 1 && zoff == 1)
			voxel->subVoxels |= (1 << SUBVOXEL_XYZ);
	}
}

void updateCulledVoxels()
{
	Item* items = getItemList();
	int len = dynList_size(items);
	for (int z = mapReigonStart[2]; z < mapReigonEnd[2]; z++)
	{
		for (int y = mapReigonStart[1]; y < mapReigonEnd[1]; y++)
		{
			for (int x = mapReigonStart[0]; x < mapReigonEnd[0]; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				voxel->subVoxels = 0;
				if (voxel->faces)
				{
					vec3 pos = {x, y, z};
					for (int i = 0; i < len; i++)
					{
						Item* item = &items[i];
						if (!isItemValid(item))
							continue;
						if (item->def->deleteIntersectingVoxelsEditor == 0)
							continue;
						if (itemIntersectsVoxel(item, pos, 1))
						{
							checkSubVoxel(item, pos, 0, 0, 0, voxel);
							checkSubVoxel(item, pos, 1, 0, 0, voxel);
							checkSubVoxel(item, pos, 0, 1, 0, voxel);
							checkSubVoxel(item, pos, 1, 1, 0, voxel);
							checkSubVoxel(item, pos, 0, 0, 1, voxel);
							checkSubVoxel(item, pos, 1, 0, 1, voxel);
							checkSubVoxel(item, pos, 0, 1, 1, voxel);
							checkSubVoxel(item, pos, 1, 1, 1, voxel);
						}
					}
				}
			}
		}
	}
}
