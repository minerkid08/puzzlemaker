#include "raycast.h"
#include "item/item.h"
#include "voxel/voxel.h"
#include <string.h>

char raycast(vec3 start, vec3 dir, float len, int flags, RaycastHit* hit)
{
	vec3 end = {start[0] + dir[0] * len, start[1] + dir[1] * len, start[2] + dir[2] * len};
	char hitAir = 0;
	for (float i = 0; i < 1.0f; i += 0.01f)
	{
		vec3 out;
		glm_vec3_lerp(start, end, i, out);
		if (flags & RAYCAST_ITEM)
		{
			Item* item = getIntersectingItem(out);
			if (item)
			{
				hit->type = RAYCAST_ITEM;
				hit->item = item;
				hit->voxel = 0;
				hit->dir = 0;
				memcpy(hit->pos, item->pos, sizeof(vec3));
				return 1;
			}
		}

		if (flags & RAYCAST_VOXEL)
		{
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

			hit->type = RAYCAST_VOXEL;
			hit->item = 0;
			hit->voxel = v;
			ivec3 ipos = {x, y, z};
			hit->dir = getVoxelSide(start, ipos, dir);
			hit->pos[0] = x;
			hit->pos[1] = y;
			hit->pos[2] = z;
			return 1;
		}
	}
	return 0;
}
