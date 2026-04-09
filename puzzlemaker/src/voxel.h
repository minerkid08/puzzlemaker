#pragma once

#include <cglm/cglm.h>

#define MAP_SIZE 20
#define VOXEL_COUNT (MAP_SIZE * MAP_SIZE * MAP_SIZE)

#define inRange(x, y, z) (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE && z >= 0 && z < MAP_SIZE)

#define normalCol (vec4){1, 1, 1, 1}
#define portalCol (vec4){1.5, 1.5, 1.5, 1}
#define selectCol (vec4){0, 1, 0, 1}

typedef struct
{
	char solid;
	char portalability[6];
} Voxel;

typedef struct
{
	ivec3 pos;
	Voxel* voxel;
	char dir;
} RaycastHit;

extern ivec3 currentVoxelPos;
extern Voxel* currentVoxel;
extern char currentDir;

void initVoxels();
void drawVoxels(vec3 pos, vec3 rot);

Voxel* getVoxel(int x, int y, int z);
Voxel* getVoxelv(ivec3 pos);

char voxelRaycast(vec3 startPos, vec3 dir, float len, RaycastHit* out);
