#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "item/item.h"
#include <math.h>
#include <string.h>

typedef struct
{
	vec3 center;
	vec3 axes[3];
	vec3 halfSize;
} OBB;

float getPlaneAxis(vec3 axis, float size, vec3 plane)
{
	vec3 tmp;
	glm_vec3_scale(axis, size, tmp);
	return fabs(glm_vec3_dot(tmp, plane));
}

char getSeparatingPlane(vec3 rPos, vec3 plane, OBB* box1, OBB* box2)
{
	return (
		fabs(glm_vec3_dot(rPos, plane)) >
		(getPlaneAxis(box1->axes[0], box1->halfSize[0], plane) + getPlaneAxis(box1->axes[1], box1->halfSize[1], plane) +
		 getPlaneAxis(box1->axes[2], box1->halfSize[2], plane) + getPlaneAxis(box2->axes[0], box2->halfSize[0], plane) +
		 getPlaneAxis(box2->axes[1], box2->halfSize[1], plane) +
		 getPlaneAxis(box2->axes[2], box2->halfSize[2], plane)));
}

char getSeparatingPlaneCross(vec3 rPos, vec3 plane1, vec3 plane2, OBB* box1, OBB* box2)
{
	vec3 plane;
	glm_vec3_cross(plane1, plane2, plane);
	return getSeparatingPlane(rPos, plane, box1, box2);
}

// test for separating planes in all 15 axes
char getCollision(OBB* box1, OBB* box2)
{
	vec3 rPos;
	glm_vec3_sub(box1->center, box2->center, rPos);

	return !(
		getSeparatingPlane(rPos, box1->axes[0], box1, box2) || getSeparatingPlane(rPos, box1->axes[1], box1, box2) ||
		getSeparatingPlane(rPos, box1->axes[2], box1, box2) || getSeparatingPlane(rPos, box2->axes[0], box1, box2) ||
		getSeparatingPlane(rPos, box2->axes[1], box1, box2) || getSeparatingPlane(rPos, box2->axes[2], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[0], box2->axes[0], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[0], box2->axes[1], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[0], box2->axes[2], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[1], box2->axes[0], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[1], box2->axes[1], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[1], box2->axes[2], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[2], box2->axes[0], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[2], box2->axes[1], box1, box2) ||
		getSeparatingPlaneCross(rPos, box1->axes[2], box2->axes[2], box1, box2));
}

void genOBB(vec3 a, vec3 b, mat4 rotMatrix, vec3 posOffset, OBB* obb)
{
	obb->halfSize[0] = (b[0] - a[0]) / 2.0f;
	obb->halfSize[1] = (b[1] - a[1]) / 2.0f;
	obb->halfSize[2] = (b[2] - a[2]) / 2.0f;

	vec3 center;
	center[0] = (b[0] + a[0]) / 2.0f;
	center[1] = (b[1] + a[1]) / 2.0f;
	center[2] = (b[2] + a[2]) / 2.0f;
	if (posOffset)
	{
		glm_mat4_mulv3(rotMatrix, center, 1, center);
		center[0] += posOffset[0];
		center[1] += posOffset[1];
		center[2] += posOffset[2];
	}
	memcpy(&obb->center, &center, sizeof(vec3));

	vec3 dirx = {1, 0, 0};
	vec3 diry = {0, 1, 0};
	vec3 dirz = {0, 0, 1};
	vec3 axis;
	glm_mat4_mulv3(rotMatrix, dirx, 1, axis);
	memcpy(obb->axes[0], axis, sizeof(vec3));
	glm_mat4_mulv3(rotMatrix, diry, 1, axis);
	memcpy(obb->axes[1], axis, sizeof(vec3));
	glm_mat4_mulv3(rotMatrix, dirz, 1, axis);
	memcpy(obb->axes[2], axis, sizeof(vec3));
}

char itemIntersectsVoxel(Item* item, vec3 pos, float size)
{
	OBB a;
	OBB b;

	mat4 rotMatrix;
	glm_mat4_identity(rotMatrix);
	vec3 posB = {pos[0] + size, pos[1] + size, pos[2] + size};
	genOBB(pos, posB, rotMatrix, 0, &a);

	vec4 quat;
	memcpy(quat, item->quat, sizeof(vec4));
	glm_quat_mat4(quat, rotMatrix);
	vec3 boundA;
	vec3 boundB;
	item->def->callbacks->getBoundingBox(item, boundA, boundB);
	boundA[0] += 0.05;
	boundA[1] += 0.05;
	boundA[2] += 0.05;
	boundB[0] -= 0.05;
	boundB[1] -= 0.05;
	boundB[2] -= 0.05;
	genOBB(boundA, boundB, rotMatrix, item->pos, &b);
	char res = getCollision(&a, &b);
	return res;
}
