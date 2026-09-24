#include "voxel/voxel.h"
#include "export/brush.h"
#include "utils.h"
#include "voxel/voxelConfig.h"

const char* getMat(Voxel* voxel, int dir)
{
	char portal = voxel->portalability[dir];
	if (dir == DIR_POS_Y)
		return (portal ? voxelConfig.whiteFloor : voxelConfig.blackFloor);

	if (dir == DIR_NEG_Y)
		return (portal ? voxelConfig.whiteCeiling : voxelConfig.blackCeiling);

	return (portal ? voxelConfig.whiteWall : voxelConfig.blackWall);
}

void exportVoxel(Voxel* voxel, ivec3 pos, ivec3 size)
{
	vec3 start = {pos[0], pos[1], pos[2]};
	vec3 end = {pos[0] + 1, pos[1] + 1, pos[2] + 1};
	if (size)
	{
		end[0] = pos[0] + size[0];
		end[1] = pos[1] + size[1];
		end[2] = pos[2] + size[2];
	}
	Brush* brush = exportCreateBrush(start, end, 0);

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];
		side->material = voxelConfig.nodraw;
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

char canMerge(Voxel* a, Voxel* b)
{
	if (a->faces != b->faces)
		return 0;

	switch (a->faces)
	{
	case (1 << DIR_POS_X):
		return a->portalability[DIR_POS_X] == b->portalability[DIR_POS_X];
	case (1 << DIR_NEG_X):
		return a->portalability[DIR_NEG_X] == b->portalability[DIR_NEG_X];
	case (1 << DIR_POS_Y):
		return a->portalability[DIR_POS_Y] == b->portalability[DIR_POS_Y];
	case (1 << DIR_NEG_Y):
		return a->portalability[DIR_NEG_Y] == b->portalability[DIR_NEG_Y];
	case (1 << DIR_POS_Z):
		return a->portalability[DIR_POS_Z] == b->portalability[DIR_POS_Z];
	case (1 << DIR_NEG_Z):
		return a->portalability[DIR_NEG_Z] == b->portalability[DIR_NEG_Z];
	default:
		return 0;
	}
}

char getVoxelDir(Voxel* v)
{
	switch (v->faces)
	{
	case (1 << DIR_POS_X):
		return DIR_POS_X;
	case (1 << DIR_NEG_X):
		return DIR_NEG_X;
	case (1 << DIR_POS_Y):
		return DIR_POS_Y;
	case (1 << DIR_NEG_Y):
		return DIR_NEG_Y;
	case (1 << DIR_POS_Z):
		return DIR_POS_Z;
	case (1 << DIR_NEG_Z):
		return DIR_NEG_Z;
	default:
		return -1;
	}
}

void scanX(Voxel* voxel, int x, int y, int z, int* width, ivec2 tile)
{
	int maxTile = max(tile[0], tile[1]);
	ivec2 tileNorm = {0, 0};
	if (tile[0] > 0)
		tileNorm[0] = 1;
	if (tile[1] > 0)
		tileNorm[1] = 1;
	while (1)
	{
		int x2 = x + *width;
		if (x2 >= MAP_SIZE)
			return;
		for (int i = 0; i < maxTile; i++)
		{
			Voxel* v2 = getVoxel(x2, y + i * tileNorm[0], z + i * tileNorm[1]);
			if (!canMerge(voxel, v2))
				return;
		}
		(*width)++;
	}
}

void scanY(Voxel* voxel, int x, int y, int z, int* width, ivec2 tile)
{
	int maxTile = max(tile[0], tile[1]);
	ivec2 tileNorm = {0, 0};
	if (tile[0] > 0)
		tileNorm[0] = 1;
	if (tile[1] > 0)
		tileNorm[1] = 1;
	while (1)
	{
		int y2 = y + *width;
		if (y2 >= MAP_SIZE)
			return;
		for (int i = 0; i < maxTile; i++)
		{
			Voxel* v2 = getVoxel(x + i * tileNorm[0], y2, z + i * tileNorm[1]);
			if (!canMerge(voxel, v2))
				return;
		}
		(*width)++;
	}
}

void scanZ(Voxel* voxel, int x, int y, int z, int* width, ivec2 tile)
{
	int maxTile = max(tile[0], tile[1]);
	ivec2 tileNorm = {0, 0};
	if (tile[0] > 0)
		tileNorm[0] = 1;
	if (tile[1] > 0)
		tileNorm[1] = 1;
	while (1)
	{
		int z2 = z + *width;
		if (z2 >= MAP_SIZE)
			return;
		for (int i = 0; i < maxTile; i++)
		{
			Voxel* v2 = getVoxel(x + i * tileNorm[0], y + i * tileNorm[1], z2);
			if (!canMerge(voxel, v2))
				return;
		}
		(*width)++;
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
	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->faces == 0)
					continue;
				char dir = getVoxelDir(voxel);
				if (dir == -1)
				{
					char isX = 1;
					char isY = 1;
					char isZ = 1;
					if (voxel->faces & (1 << DIR_POS_X) || voxel->faces & (1 << DIR_NEG_X))
						isX = 0;
					if (voxel->faces & (1 << DIR_POS_Y) || voxel->faces & (1 << DIR_NEG_Y))
						isY = 0;
					if (voxel->faces & (1 << DIR_POS_Z) || voxel->faces & (1 << DIR_NEG_Z))
						isZ = 0;

					ivec3 pos = {x, y, z};
					if (!isX && !isY && !isZ)
					{
						ivec3 size = {1, 1, 1};
						exportVoxel(voxel, pos, size);
						continue;
					}

					if (isX)
					{
						int width = 1;
						while (1)
						{
							int x2 = x + width;
							if (x2 >= MAP_SIZE)
								break;
							Voxel* v2 = getVoxel(x2, y, z);
							if (voxel->faces != v2->faces)
								break;
							width++;
						}
						ivec3 size = {width, 1, 1};
						exportVoxel(voxel, pos, size);
						for (int x2 = 0; x2 < width; x2++)
							getVoxel(x + x2, y, z)->faces = 0;
					}
					if (isY)
					{
						int width = 1;
						while (1)
						{
							int y2 = y + width;
							if (y2 >= MAP_SIZE)
								break;
							Voxel* v2 = getVoxel(x, y2, z);
							if (voxel->faces != v2->faces)
								break;
							width++;
						}
						ivec3 size = {1, width, 1};
						exportVoxel(voxel, pos, size);
						for (int y2 = 0; y2 < width; y2++)
							getVoxel(x, y + y2, z)->faces = 0;
					}
					if (isZ)
					{
						int width = 1;
						while (1)
						{
							int z2 = z + width;
							if (z2 >= MAP_SIZE)
								break;
							Voxel* v2 = getVoxel(x, y, z2);
							if (voxel->faces != v2->faces)
								break;
							width++;
						}
						ivec3 size = {1, 1, width};
						exportVoxel(voxel, pos, size);
						for (int z2 = 0; z2 < width; z2++)
							getVoxel(x, y, z + z2)->faces = 0;
					}
				}

				if (dir == DIR_POS_X || dir == DIR_NEG_X)
				{
					int height = 1;
					ivec2 tile = {1, 0};
					scanY(voxel, x, y, z, &height, tile);

					int length = 1;
					tile[0] = 0;
					tile[1] = height;
					scanZ(voxel, x, y, z, &length, tile);
					for (int z2 = 0; z2 < length; z2++)
					{
						for (int y2 = 0; y2 < height; y2++)
							getVoxel(x, y + y2, z + z2)->faces = 0;
					}
					ivec3 pos = {x, y, z};
					ivec3 size = {1, height, length};
					exportVoxel(voxel, pos, size);
				}

				if (dir == DIR_POS_Y || dir == DIR_NEG_Y)
				{
					int width = 1;
					ivec2 tile = {1, 0};
					scanX(voxel, x, y, z, &width, tile);

					int length = 1;
					tile[0] = width;
					tile[1] = 0;
					scanZ(voxel, x, y, z, &length, tile);
					for (int z2 = 0; z2 < length; z2++)
					{
						for (int x2 = 0; x2 < width; x2++)
							getVoxel(x + x2, y, z + z2)->faces = 0;
					}
					ivec3 pos = {x, y, z};
					ivec3 size = {width, 1, length};
					exportVoxel(voxel, pos, size);
				}

				if (dir == DIR_POS_Z || dir == DIR_NEG_Z)
				{
					int height = 1;
					ivec2 tile = {0, 1};
					scanY(voxel, x, y, z, &height, tile);

					int length = 1;
					tile[0] = height;
					tile[1] = 0;
					scanX(voxel, x, y, z, &length, tile);
					for (int y2 = 0; y2 < height; y2++)
					{
						for (int x2 = 0; x2 < length; x2++)
							getVoxel(x + x2, y + y2, z)->faces = 0;
					}
					ivec3 pos = {x, y, z};
					ivec3 size = {length, height, 1};
					exportVoxel(voxel, pos, size);
				}
			}
		}
	}
}
