#pragma once

#include "cglm/vec3.h"
#include "item/item.h"

typedef struct
{
	vec3 center;
	vec3 axes[3];
	vec3 halfSize;
} OBB;

char getCollision(OBB* box1, OBB* box2);
void genOBB(vec3 a, vec3 b, mat4 rotMatrix, vec3 posOffset, OBB* obb);
char itemIntersectsVoxel(Item* item, vec3 pos, float size);
