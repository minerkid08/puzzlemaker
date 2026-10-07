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

void drawVoxels(vec3 cameraPos, vec3 cameraRot)
{
	bindVoxelTextures(voxelConfig.blackEditor, voxelConfig.whiteEditor, voxelConfig.blackMiniEditor,
					  voxelConfig.whiteMiniEditor);

	mat4 camMat;
	glm_mat4_identity(camMat);
	glm_translate(camMat, cameraPos);
	glm_rotate_z(camMat, cameraRot[2], camMat);
	glm_rotate_y(camMat, cameraRot[1], camMat);
	glm_rotate_x(camMat, cameraRot[0], camMat);

	glm_mat4_inv_fast(camMat, camMat);

	setCamMat(camMat);

	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->solid)
				{
					ivec3 pos = {x, y, z};

					vec4 tint = {1, 1, 1, 1};

					if (z + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y, z + 1);
						if (!v2->solid)
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
						Voxel* v2 = getVoxel(x, y, z - 1);
						if (!v2->solid)
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
						Voxel* v2 = getVoxel(x + 1, y, z);
						if (!v2->solid)
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
						Voxel* v2 = getVoxel(x - 1, y, z);
						if (!v2->solid)
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
						Voxel* v2 = getVoxel(x, y + 1, z);
						if (!v2->solid)
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
						Voxel* v2 = getVoxel(x, y - 1, z);
						if (!v2->solid)
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
