#pragma once

#include "cglm/types.h"
#include <cglm/cglm.h>

#define MAP_SIZE 256
#define VOXEL_COUNT (MAP_SIZE * MAP_SIZE * MAP_SIZE)

#define SUBVOXEL_xyz 0
#define SUBVOXEL_Xyz 1
#define SUBVOXEL_xYz 2
#define SUBVOXEL_XYz 3
#define SUBVOXEL_xyZ 4
#define SUBVOXEL_XyZ 5
#define SUBVOXEL_xYZ 6
#define SUBVOXEL_XYZ 7

#define inRange(x, y, z) (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE && z >= 0 && z < MAP_SIZE)
#define canPush(x, y, z) (x > 0 && x < MAP_SIZE - 1 && y > 0 && y < MAP_SIZE - 1 && z > 0 && z < MAP_SIZE - 1)

extern ivec3 mapReigonStart;
extern ivec3 mapReigonEnd;

typedef struct
{
	char solid;
	char portalability[6];
  char faces;
  char subVoxels;
} Voxel;

void initVoxels();
void initVoxelRenderer();
void drawVoxels();
char pointInRange(ivec3 point, ivec3 boundA, ivec3 boundB);
char isSelection2d();

Voxel* getVoxel(int x, int y, int z);
Voxel* getVoxelv(ivec3 pos);

int getVoxelSide(vec3 start, ivec3 pos, vec3 dir, vec3 pos2);
char voxelRaycast(vec3 pos, vec3 dir, ivec3 out);
void updateVoxelFaces();
void updateCulledVoxels();
