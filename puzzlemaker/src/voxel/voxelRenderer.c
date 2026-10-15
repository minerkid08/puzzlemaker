#include "cglm/types.h"
#include "renderer/renderer.h"
#include "selection.h"
#include "utils.h"
#include "voxel/voxel.h"
#include "voxel/voxelConfig.h"
#include <cglm/cglm.h>

static inline char isSelected(int x, int y, int z, char dir)
{
	ivec3 pos = {x, y, z};
	if (dir == selection.voxelDir || selection.voxelDir == DIR_NONE)
		return pointInRange(pos, selection.voxelPos, selection.voxel2Pos);
	return 0;
}

void drawSubVoxels(int x, int y, int z, Voxel* voxel)
{
	vec4 tint = {1, 1, 1, 1};
	for (int i = 0; i < 8; i++)
	{
		if ((voxel->subVoxels & (1 << i)) == 0)
		{
			float xoff = (i & 1) ? 0.5f : 0.0f;
			float yoff = (i & 2) ? 0.5f : 0.0f;
			float zoff = (i & 4) ? 0.5f : 0.0f;
			vec3 start = {x + xoff, y + yoff, z + zoff};
			vec3 end = {x + 0.5f + xoff, y + 0.5f + yoff, z + 0.5f + zoff};

			float x2 = x + xoff;
			float y2 = y + yoff;
			float z2 = z + zoff;
			float x3 = x + xoff + 0.5f;
			float y3 = y + yoff + 0.5f;
			float z3 = z + zoff + 0.5f;

			if (xoff == 0.5f)
			{
				if (voxel->faces & (1 << DIR_POS_X))
				{
					if (isSelected(x + 1, y, z, DIR_POS_X))
					{
						tint[0] = 0;
						tint[2] = 0;
					}
					vec3 verts[4] = {{x + 1, y2, z2}, {x + 1, y3, z2}, {x + 1, y2, z3}, {x + 1, y3, z3}};
					drawVerts2(verts, tint, voxel->portalability[DIR_POS_X] | 2);
					tint[0] = 1;
					tint[2] = 1;
				}
			}

			if (xoff == 0.0f)
			{
				if (voxel->faces & (1 << DIR_NEG_X))
				{
					if (isSelected(x - 1, y, z, DIR_NEG_X))
					{
						tint[0] = 0;
						tint[2] = 0;
					}
					vec3 verts[4] = {{x, y2, z2}, {x, y2, z3}, {x, y3, z2}, {x, y3, z3}};
					drawVerts2(verts, tint, voxel->portalability[DIR_NEG_X] | 2);
					tint[0] = 1;
					tint[2] = 1;
				}
			}
			if (yoff == 0.5f)
			{
				if (voxel->faces & (1 << DIR_POS_Y))
				{
					if (isSelected(x, y + 1, z, DIR_POS_Y))
					{
						tint[0] = 0;
						tint[2] = 0;
					}
					vec3 verts[4] = {{x2, y + 1, z2}, {x2, y + 1, z3}, {x3, y + 1, z2}, {x3, y + 1, z3}};
					drawVerts2(verts, tint, voxel->portalability[DIR_POS_Y] | 2);
					tint[0] = 1;
					tint[2] = 1;
				}
			}

			if (yoff == 0.0f)
			{
				if (voxel->faces & (1 << DIR_NEG_Y))
				{
					if (isSelected(x, y - 1, z, DIR_NEG_Y))
					{
						tint[0] = 0;
						tint[2] = 0;
					}
					vec3 verts[4] = {{x2, y, z2}, {x3, y, z2}, {x2, y, z3}, {x3, y, z3}};
					drawVerts2(verts, tint, voxel->portalability[DIR_NEG_Y] | 2);
					tint[0] = 1;
					tint[2] = 1;
				}
			}
			if (zoff == 0.0f)
			{
				if (voxel->faces & (1 << DIR_NEG_Z))
				{
					if (isSelected(x, y, z - 1, DIR_NEG_Z))
					{
						tint[0] = 0;
						tint[2] = 0;
					}
					vec3 verts[4] = {{x2, y2, z}, {x2, y3, z}, {x3, y2, z}, {x3, y3, z}};
					drawVerts2(verts, tint, voxel->portalability[DIR_NEG_Z] | 2);
					tint[0] = 1;
					tint[2] = 1;
				}
			}
			if (zoff == 0.5f)
			{
				if (voxel->faces & (1 << DIR_POS_Z))
				{
					if (isSelected(x, y, z + 1, DIR_POS_Z))
					{
						tint[0] = 0;
						tint[2] = 0;
					}
					vec3 verts[4] = {{x2, y2, z + 1}, {x3, y2, z + 1}, {x2, y3, z + 1}, {x3, y3, z + 1}};
					drawVerts2(verts, tint, voxel->portalability[DIR_POS_Z] | 2);
					tint[0] = 1;
					tint[2] = 1;
				}
			}
		}
	}
}

void drawVoxels()
{
	bindVoxelTextures(voxelConfig.blackEditor, voxelConfig.whiteEditor, voxelConfig.blackMiniEditor,
					  voxelConfig.whiteMiniEditor);

	for (int z = mapReigonStart[2]; z < mapReigonEnd[2]; z++)
	{
		for (int y = mapReigonStart[1]; y < mapReigonEnd[1]; y++)
		{
			for (int x = mapReigonStart[0]; x < mapReigonEnd[0]; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->solid)
				{
					ivec3 pos = {x, y, z};

					vec4 tint = {1, 1, 1, 1};

					if (voxel->subVoxels)
					{
						drawSubVoxels(x, y, z, voxel);
						continue;
					}

					if (z + 1 < MAP_SIZE)
					{
						if (voxel->faces & (1 << DIR_POS_Z))
						{
							if (isSelected(x, y, z + 1, DIR_POS_Z))
							{
								tint[0] = 0;
								tint[2] = 0;
							}
							vec3 verts[4] = {
								{x, y, z + 1}, {x + 1, y, z + 1}, {x, y + 1, z + 1}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, tint, voxel->portalability[DIR_POS_Z]);
							tint[0] = 1;
							tint[2] = 1;
						}
					}

					if (z - 1 >= 0)
					{
						if (voxel->faces & (1 << DIR_NEG_Z))
						{
							if (isSelected(x, y, z - 1, DIR_NEG_Z))
							{
								tint[0] = 0;
								tint[2] = 0;
							}
							vec3 verts[4] = {{x, y, z}, {x, y + 1, z}, {x + 1, y, z}, {x + 1, y + 1, z}};
							drawVerts(verts, tint, voxel->portalability[DIR_NEG_Z]);
							tint[0] = 1;
							tint[2] = 1;
						}
					}

					if (x + 1 < MAP_SIZE)
					{
						if (voxel->faces & (1 << DIR_POS_X))
						{
							if (isSelected(x + 1, y, z, DIR_POS_X))
							{
								tint[0] = 0;
								tint[2] = 0;
							}
							vec3 verts[4] = {
								{x + 1, y, z}, {x + 1, y + 1, z}, {x + 1, y, z + 1}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, tint, voxel->portalability[DIR_POS_X]);
							tint[0] = 1;
							tint[2] = 1;
						}
					}

					if (x - 1 >= 0)
					{
						if (voxel->faces & (1 << DIR_NEG_X))
						{
							if (isSelected(x - 1, y, z, DIR_NEG_X))
							{
								tint[0] = 0;
								tint[2] = 0;
							}
							vec3 verts[4] = {{x, y, z}, {x, y, z + 1}, {x, y + 1, z}, {x, y + 1, z + 1}};
							drawVerts(verts, tint, voxel->portalability[DIR_NEG_X]);
							tint[0] = 1;
							tint[2] = 1;
						}
					}

					if (y + 1 < MAP_SIZE)
					{
						if (voxel->faces & (1 << DIR_POS_Y))
						{
							if (isSelected(x, y + 1, z, DIR_POS_Y))
							{
								tint[0] = 0;
								tint[2] = 0;
							}
							vec3 verts[4] = {
								{x, y + 1, z}, {x, y + 1, z + 1}, {x + 1, y + 1, z}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, tint, voxel->portalability[DIR_POS_Y]);
							tint[0] = 1;
							tint[2] = 1;
						}
					}

					if (y - 1 >= 0)
					{
						if (voxel->faces & (1 << DIR_NEG_Y))
						{
							if (isSelected(x, y - 1, z, DIR_NEG_Y))
							{
								tint[0] = 0;
								tint[2] = 0;
							}
							vec3 verts[4] = {{x, y, z}, {x + 1, y, z}, {x, y, z + 1}, {x + 1, y, z + 1}};
							drawVerts(verts, tint, voxel->portalability[DIR_NEG_Y]);
							tint[0] = 1;
							tint[2] = 1;
						}
					}
				}
			}
		}
	}
}
