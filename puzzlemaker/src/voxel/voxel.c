#include "voxel.h"

#include "cglm/vec3.h"
#include "utils.h"
#include <string.h>

Voxel voxels[MAP_SIZE * MAP_SIZE * MAP_SIZE];

ivec3 currentVoxelPos;
ivec3 currentVoxel2Pos;
Voxel* currentVoxel;
char currentDir;

char isSelection2d()
{
	return currentVoxelPos[0] == currentVoxel2Pos[0] || currentVoxelPos[1] == currentVoxel2Pos[1] ||
		   currentVoxelPos[2] == currentVoxel2Pos[2];
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
	initVoxelRenderer();

	memset(voxels, 0, sizeof(voxels));
	for (int i = 0; i < VOXEL_COUNT; i++)
	{
		voxels[i].solid = 1;
	}

	for (int z = 1; z < 7; z++)
	{
		for (int y = 1; y < 5; y++)
		{
			for (int x = 1; x < 7; x++)
			{
				getVoxel(x, y, z)->solid = 0;
			}
		}
	}
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

int getVoxelSide(vec3 start, ivec3 pos2, vec3 dir)
{
	int x = pos2[0];
	int y = pos2[1];
	int z = pos2[2];

	vec3 pos;

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
