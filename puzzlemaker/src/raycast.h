#include <cglm/cglm.h>
#include "voxel/voxel.h"
#include "item/item.h"

#define RAYCAST_NONE 0
#define RAYCAST_VOXEL 1
#define RAYCAST_ITEM 2

typedef struct
{
	vec3 pos;
	int type;
	Item* item;
	Voxel* voxel;
	char dir;
} RaycastHit;

char raycast(vec3 start, vec3 dir, float len, int flags, RaycastHit* hit);
