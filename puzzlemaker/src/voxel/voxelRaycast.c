#include "cglm/types.h"
#include "voxel/voxel.h"
#include <math.h>

char checkDir(char dir, int pos)
{
	if (pos >= 0 && pos < MAP_SIZE)
		return 1;
	if (dir > 0 && pos < 0)
		return 1;
	if (dir < 0 && pos >= MAP_SIZE)
		return 1;
	return 0;
}

char voxelRaycast(vec3 pos, vec3 dir, ivec3 out)
{
	// which box of the map we're in
	int mapX = floorf(pos[0]);
	int mapY = floorf(pos[1]);
	int mapZ = floorf(pos[2]);

	// length of ray from current position to next x or y-side
	float sideDistX = 0.0f;
	float sideDistY = 0.0f;
	float sideDistZ = 0.0f;

	// length of ray from one x or y-side to next x or y-side
	float deltaDistX = (dir[0] == 0) ? 1e30 : fabsf(1 / dir[0]);
	float deltaDistY = (dir[1] == 0) ? 1e30 : fabsf(1 / dir[1]);
	float deltaDistZ = (dir[2] == 0) ? 1e30 : fabsf(1 / dir[2]);
	float perpWallDist;

	// what direction to step in x or y-direction (either +1 or -1)
	char stepX;
	char stepY;
	char stepZ;
	char dirX;
	char dirY;
	char dirZ;

	int hit = 0; // was there a wall hit?

	if (dir[0] < 0)
	{
		stepX = -1;
		sideDistX = (pos[0] - mapX) * deltaDistX;
	}
	else
	{
		stepX = 1;
		sideDistX = (mapX + 1.0 - pos[0]) * deltaDistX;
	}
	if (dir[1] < 0)
	{
		stepY = -1;
		sideDistY = (pos[1] - mapY) * deltaDistY;
	}
	else
	{
		stepY = 1;
		sideDistY = (mapY + 1.0 - pos[1]) * deltaDistY;
	}
	if (dir[2] < 0)
	{
		stepZ = -1;
		sideDistZ = (pos[2] - mapZ) * deltaDistZ;
	}
	else
	{
		stepZ = 1;
		sideDistZ = (mapZ + 1.0 - pos[2]) * deltaDistZ;
	}

	char hitAir = 0;

	while (hit == 0)
	{
		// jump to next map square, either in x-direction, or in y-direction
		if (sideDistX < sideDistY)
		{
			if (sideDistX < sideDistZ)
			{
				sideDistX += deltaDistX;
				mapX += stepX;
				if (checkDir(stepX, mapX) == 0)
					return 0;
			}
			else
			{
				sideDistZ += deltaDistZ;
				mapZ += stepZ;
				if (checkDir(stepZ, mapZ) == 0)
					return 0;
			}
		}
		else
		{
			if (sideDistY < sideDistZ)
			{
				sideDistY += deltaDistY;
				mapY += stepY;
				if (checkDir(stepY, mapY) == 0)
					return 0;
			}
			else
			{
				sideDistZ += deltaDistZ;
				mapZ += stepZ;
				if (checkDir(stepZ, mapZ) == 0)
					return 0;
			}
		}
		// Check if ray has hit a wall
		char solid = 1;
		if (inRange(mapX, mapY, mapZ))
			solid = getVoxel(mapX, mapY, mapZ)->solid;
		if (hitAir)
		{
			if (solid)
				break;
		}
		else
		{
			if (solid == 0)
				hitAir = 1;
		}
	}
	out[0] = mapX;
	out[1] = mapY;
	out[2] = mapZ;
	return 1;
}
