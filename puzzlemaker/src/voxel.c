#include "voxel.h"

#include "cglm/vec3.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "utils.h"
#include <string.h>

#define col(dir) (voxel == currentVoxel && dir == currentDir ? selectCol : (voxel->portalability[dir] ? portalCol : normalCol))

Voxel voxels[MAP_SIZE * MAP_SIZE * MAP_SIZE];

ivec3 currentVoxelPos;
ivec3 currentVoxel2Pos;
Voxel* currentVoxel;
char currentDir;

static unsigned int texture;

void initVoxels()
{
  texture = loadTexture("wall.png");

	memset(voxels, 0, sizeof(voxels));
	for (int i = 0; i < VOXEL_COUNT; i++)
	{
		voxels[i].solid = 1;
	}

	getVoxel(2, 2, 2)->solid = 0;
	getVoxel(2, 3, 2)->solid = 0;
	getVoxel(2, 2, 3)->solid = 0;
	getVoxel(2, 3, 3)->solid = 0;

	getVoxel(3, 2, 2)->solid = 0;
	getVoxel(3, 3, 2)->solid = 0;
	getVoxel(3, 2, 3)->solid = 0;
	getVoxel(3, 3, 3)->solid = 0;
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

	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->solid)
				{
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

char voxelRaycast(vec3 start, vec3 dir, float len, RaycastHit* hit)
{
	// vec3 dir;
	// glm_normalize_to(dir2, dir);
	vec3 end = {start[0] + dir[0] * len, start[1] + dir[1] * len, start[2] + dir[2] * len};

	char hitAir = 0;

	for (float i = 0; i < 1.0f; i += 0.01f)
	{
		vec3 out;
		glm_vec3_lerp(start, end, i, out);
		int x = floorf(out[0]);
		int y = floorf(out[1]);
		int z = floorf(out[2]);

		if (x < 0 || x >= MAP_SIZE)
			continue;
		if (y < 0 || y >= MAP_SIZE)
			continue;
		if (z < 0 || z >= MAP_SIZE)
			continue;

		Voxel* v = getVoxel(x, y, z);
		if (!v->solid)
		{
			hitAir = 1;
			continue;
		}

		if (!hitAir)
			continue;

		hit->pos[0] = x;
		hit->pos[1] = y;
		hit->pos[2] = z;
		hit->voxel = v;

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

		vec3 pos;
		pos[0] = (tmin * dir[0]) + start[0];
		pos[1] = (tmin * dir[1]) + start[1];
		pos[2] = (tmin * dir[2]) + start[2];

		if (absf(pos[0] - x) < 0.0001f)
		{
			hit->dir = DIR_NEG_X;
			return 1;
		}
		if (absf(pos[0] - x) > 0.9999f)
		{
			hit->dir = DIR_POS_X;
			return 1;
		}
		if (absf(pos[1] - y) < 0.0001f)
		{
			hit->dir = DIR_NEG_Y;
			return 1;
		}
		if (absf(pos[1] - y) > 0.9999f)
		{
			hit->dir = DIR_POS_Y;
			return 1;
		}
		if (absf(pos[2] - z) < 0.0001f)
		{
			hit->dir = DIR_NEG_Z;
			return 1;
		}
		if (absf(pos[2] - z) > 0.9999f)
		{
			hit->dir = DIR_POS_Z;
			return 1;
		}
	}
	return 0;
}
