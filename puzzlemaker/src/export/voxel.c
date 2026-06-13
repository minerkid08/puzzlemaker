#include "voxel/voxel.h"
#include "export/brush.h"
#include "utils.h"

const char* matNodraw = "TOOLS/TOOLSNODRAW";
const char* matBlackFloor = "METAL/BLACK_FLOOR_METAL_001C";
const char* matBlackWall = "METAL/BLACK_WALL_METAL_002A";
const char* matBlackCeiling = "METAL/BLACK_CEILING_METAL_001A";
const char* matWhiteFloor = "TILE/WHITE_FLOOR_TILE002A";
const char* matWhiteWall = "TILE/WHITE_WALL_STATE";
const char* matWhiteCeiling = "TILE/WHITE_CEILING_TILE002A";

const char* getMat(Voxel* voxel, int dir)
{
	char portal = voxel->portalability[dir];
	if (dir == DIR_POS_Y)
		return (portal ? matWhiteFloor : matBlackFloor);

	if (dir == DIR_NEG_Y)
		return (portal ? matWhiteCeiling : matBlackCeiling);

	return (portal ? matWhiteWall : matBlackWall);
}

void exportVoxel(Voxel* voxel, ivec3 pos)
{
	vec3 start = {pos[0], pos[1], pos[2]};
	vec3 end = {pos[0] + 1, pos[1] + 1, pos[2] + 1};
	Brush* brush = exportCreateBrush(start, end);

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];
		side->material = matNodraw;
	}

	int x = pos[0];
	int y = pos[1];
	int z = pos[2];

	if (z + 1 < MAP_SIZE)
	{
		Voxel* v2 = getVoxel(x, y, z + 1);
		if (!v2->solid)
		{
			Side* side = &brush->sides[DIR_POS_Z];
			side->material = getMat(voxel, DIR_POS_Z);
		}
	}

	if (z - 1 >= 0)
	{
		Voxel* v2 = getVoxel(x, y, z - 1);
		if (!v2->solid)
		{
			Side* side = &brush->sides[DIR_NEG_Z];
			side->material = getMat(voxel, DIR_NEG_Z);
		}
	}

	if (x + 1 < MAP_SIZE)
	{
		Voxel* v2 = getVoxel(x + 1, y, z);
		if (!v2->solid)
		{
			Side* side = &brush->sides[DIR_POS_X];
			side->material = getMat(voxel, DIR_POS_X);
		}
	}

	if (x - 1 >= 0)
	{
		Voxel* v2 = getVoxel(x - 1, y, z);
		if (!v2->solid)
		{
			Side* side = &brush->sides[DIR_NEG_X];
			side->material = getMat(voxel, DIR_NEG_X);
		}
	}

	if (y + 1 < MAP_SIZE)
	{
		Voxel* v2 = getVoxel(x, y + 1, z);
		if (!v2->solid)
		{
			Side* side = &brush->sides[DIR_POS_Y];
			side->material = getMat(voxel, DIR_POS_Y);
		}
	}

	if (y - 1 >= 0)
	{
		Voxel* v2 = getVoxel(x, y - 1, z);
		if (!v2->solid)
		{
			Side* side = &brush->sides[DIR_NEG_Y];
			side->material = getMat(voxel, DIR_NEG_Y);
		}
	}
}

void exportVoxels()
{
	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->solid)
				{
					char shouldExport = 0;
					if (z + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y, z + 1);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (z - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y, z - 1);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (x + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x + 1, y, z);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (x - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x - 1, y, z);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (y + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y + 1, z);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (y - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y - 1, z);
						if (!v2->solid)
							shouldExport = 1;
					}
					if (shouldExport)
					{
						ivec3 pos = {x, y, z};
						exportVoxel(voxel, pos);
					}
				}
			}
		}
	}
}
