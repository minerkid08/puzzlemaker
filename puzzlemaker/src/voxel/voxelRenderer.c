#include "renderer/renderer.h"
#include <cglm/cglm.h>
#include "voxel/voxel.h"
#include "utils.h"
#include "renderer/texture.h"

static unsigned int texture;

void initVoxelRenderer()
{
	texture = loadTexture("wall.png");
}

#define col(dir)                                                                                                       \
	(selected && (dir == currentDir || not2d) ? selectCol : (voxel->portalability[dir] ? portalCol : normalCol))

void drawVoxels(vec3 cameraPos, vec3 cameraRot)
{
	bindTexture(texture);

	mat4 camMat;
	glm_mat4_identity(camMat);
	glm_translate(camMat, cameraPos);
	glm_rotate_z(camMat, cameraRot[2], camMat);
	glm_rotate_y(camMat, cameraRot[1], camMat);
	glm_rotate_x(camMat, cameraRot[0], camMat);

	glm_mat4_inv_fast(camMat, camMat);

	setCamMat(camMat);

	char multiselect = (currentVoxel2Pos[0] != -1 && currentVoxel2Pos[1] != -1 && currentVoxel2Pos[2] != -1);
	char not2d = 0;
	if (multiselect)
	{
		not2d = !isSelection2d();
	}

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

					char selected;
					if (multiselect)
						selected = pointInRange(pos, currentVoxelPos, currentVoxel2Pos);
					else
						selected = (voxel == currentVoxel);

					if (z + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y, z + 1);
						if (!v2->solid)
						{
							vec3 verts[4] = {
								{x, y, z + 1}, {x + 1, y, z + 1}, {x, y + 1, z + 1}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, col(DIR_POS_Z));
						}
					}

					if (z - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y, z - 1);
						if (!v2->solid)
						{
							vec3 verts[4] = {{x, y, z}, {x, y + 1, z}, {x + 1, y, z}, {x + 1, y + 1, z}};
							drawVerts(verts, col(DIR_NEG_Z));
						}
					}

					if (x + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x + 1, y, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {
								{x + 1, y, z}, {x + 1, y + 1, z}, {x + 1, y, z + 1}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, col(DIR_POS_X));
						}
					}

					if (x - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x - 1, y, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {{x, y, z}, {x, y, z + 1}, {x, y + 1, z}, {x, y + 1, z + 1}};
							drawVerts(verts, col(DIR_NEG_X));
						}
					}

					if (y + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y + 1, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {
								{x, y + 1, z}, {x, y + 1, z + 1}, {x + 1, y + 1, z}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, col(DIR_POS_Y));
						}
					}

					if (y - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y - 1, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {{x, y, z}, {x + 1, y, z}, {x, y, z + 1}, {x + 1, y, z + 1}};
							drawVerts(verts, col(DIR_NEG_Y));
						}
					}
				}
			}
		}
	}
}
