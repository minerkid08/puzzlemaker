#include "raycast.h"
#include "antline/antline.h"
#include "item/item.h"
#include "voxel/voxel.h"
#include <string.h>

char raycast(vec3 start, vec3 dir, float len, int flags, RaycastHit* hit, Item** ignore)
{
	float itemDist = 9999;
	vec3 end = {start[0] + dir[0] * len, start[1] + dir[1] * len, start[2] + dir[2] * len};
	char rtn = 0;
	for (float i = 0; i < 1.0f; i += 0.001f)
	{
		vec3 out;
		glm_vec3_lerp(start, end, i, out);
		if (flags & RAYCAST_ITEM)
		{
			Item* item = getIntersectingItem(out, ignore);
			if (item)
			{
				hit->type = RAYCAST_ITEM;
				hit->item = item;
				hit->voxel = 0;
				hit->dir = 0;
				memcpy(hit->pos, item->pos, sizeof(vec3));
				rtn = 1;
				itemDist = i * len;
				goto end;
			}
		}

		if (flags & RAYCAST_ANTLINE)
		{
			AntlineSegment* seg = 0;
			Antline* antline = getIntersectingAntline(out, &seg);
			if (antline)
			{
				if (seg)
				{
					hit->type = RAYCAST_ANTLINE;
					hit->antline = antline;
					hit->antlineSeg = seg;
					hit->voxel = 0;
					hit->item = 0;
					hit->dir = 0;
					memcpy(hit->pos, antline->baseSegment.pos, sizeof(vec3));
					rtn = 1;
					itemDist = i * len;
					goto end;
				}
				hit->type = RAYCAST_ANTLINE;
				hit->antline = antline;
				hit->antlineSeg = &antline->baseSegment;
				hit->voxel = 0;
				hit->item = 0;
				hit->dir = 0;
				memcpy(hit->pos, antline->baseSegment.pos, sizeof(vec3));
				rtn = 1;
				itemDist = i * len;
				goto end;
			}
		}
	}
end:
	itemDist = itemDist * itemDist;
	if (flags & RAYCAST_VOXEL)
	{
		ivec3 ipos;
		if (voxelRaycast(start, dir, ipos))
		{
			vec3 pos2;
			hit->dir = getVoxelSide(start, ipos, dir, pos2);
			float x = pos2[0] - start[0];
			float y = pos2[1] - start[1];
			float z = pos2[2] - start[2];
			float voxelDist = x * x + y * y + z * z;
			if(itemDist < voxelDist && rtn)
				return 1;
			hit->type = RAYCAST_VOXEL;
			hit->item = 0;
			hit->voxel = getVoxel(ipos[0], ipos[1], ipos[2]);
			hit->pos[0] = pos2[0];
			hit->pos[1] = pos2[1];
			hit->pos[2] = pos2[2];
			hit->ipos[0] = ipos[0];
			hit->ipos[1] = ipos[1];
			hit->ipos[2] = ipos[2];
			return 1;
		}
		return rtn;
	}
	else
		return rtn;
}
