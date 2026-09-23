#include "item.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "item/entityItem.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "jsonUtils.h"
#include "raycast.h"
#include "renderer/debug.h"

#include <cjson.h>
#include <dynList.h>
#include <string.h>

extern Item* itemList;

char moving = 0;

void drawItems()
{
	int len = dynList_size(itemList);
	for (int i = 0; i < len; i++)
	{
		Item* item = &itemList[i];
		if (item->index == -1)
			continue;
		if (item->def->type == ITEM_TYPE_ENTITY)
			entityItemRender(item);
		if (item->def->type == ITEM_TYPE_PANEL)
			panelItemRender(item);
		if (item->def->type == ITEM_TYPE_VOLUME)
			volumeItemRender(item);
		// drawDebugRect(item->def->bound1, item->def->bound2);
	}
}

Item* getIntersectingItem(vec3 pos)
{
	int count = dynList_size(itemList);

	for (int j = 0; j < count; j++)
	{
		Item* item = &itemList[j];
		vec4 bound1;
		vec4 bound2;

		mat4 transform;
		memcpy(transform, item->transform, sizeof(mat4));
		memcpy(bound1, item->def->bound1, sizeof(vec4));
		memcpy(bound2, item->def->bound2, sizeof(vec4));
		glm_mat4_mulv(transform, bound1, bound1);
		glm_mat4_mulv(transform, bound2, bound2);

		vec3 p1;
		memcpy(p1, bound1, 3 * sizeof(float));

		vec3 p2;
		memcpy(p2, bound1, 3 * sizeof(float));
		p2[2] = bound2[2];

		vec3 p4;
		memcpy(p4, bound1, 3 * sizeof(float));
		p4[0] = bound2[0];

		vec3 p5;
		memcpy(p5, bound1, 3 * sizeof(float));
		p5[1] = bound2[1];

		vec3 i;
		vec3 j;
		vec3 k;
		vec3 v;

		glm_vec3_sub(p2, p1, i);
		glm_vec3_sub(p4, p1, j);
		glm_vec3_sub(p5, p1, k);
		glm_vec3_sub(pos, p1, v);

		float a = glm_vec3_dot(v, i);
		float b = glm_vec3_dot(i, i);
		float c = glm_vec3_dot(v, j);
		float d = glm_vec3_dot(j, j);
		float e = glm_vec3_dot(v, k);
		float f = glm_vec3_dot(k, k);

		if (0 < a && a < b && 0 < c && c < d && 0 < e && e < f)
			return item;
	}
	return 0;
}

void getEulerAngles(vec4 quat, vec3 out)
{
	double r11 = -2 * (quat[0] * quat[2] - quat[3] * quat[1]);
	double r12 = quat[3] * quat[3] + quat[0] * quat[0] - quat[1] * quat[1] - quat[2] * quat[2];
	double r21 = 2 * (quat[0] * quat[1] + quat[3] * quat[2]);
	double r31 = -2 * (quat[1] * quat[2] - quat[3] * quat[0]);
	double r32 = quat[3] * quat[3] - quat[0] * quat[0] + quat[1] * quat[1] - quat[2] * quat[2];
	out[0] = atan2(r31, r32);
	out[1] = asin(r21);
	out[2] = atan2(r11, r12);
}

void updateItemTransform(Item* item)
{
	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, item->pos);

	mat4 rotMat;
	vec4 itemQuat;
	memcpy(itemQuat, item->quat, sizeof(vec4));
	glm_quat_mat4(itemQuat, rotMat);

	vec3 dir;
  //glm_euler_angles(rotMat, dir);
	getEulerAngles(itemQuat, dir);
	item->dir[0] = glm_deg(dir[0]);
	item->dir[1] = glm_deg(dir[1]);
	item->dir[2] = glm_deg(dir[2]);

	glm_mat4_mul(transform, rotMat, transform);

	memcpy(item->transform, transform, sizeof(mat4));
}

void updateItemTransform2(Item* item)
{
	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, item->pos);

	mat4 rotMat;
	vec4 itemQuat;
	memcpy(itemQuat, item->quat, sizeof(vec4));
	glm_quat_mat4(itemQuat, rotMat);

	printf("%.2f, %.2f, %.2f, %.2f\n", rotMat[0][0], rotMat[0][1], rotMat[0][2], rotMat[0][3]);
	printf("%.2f, %.2f, %.2f, %.2f\n", rotMat[1][0], rotMat[1][1], rotMat[1][2], rotMat[1][3]);
	printf("%.2f, %.2f, %.2f, %.2f\n", rotMat[2][0], rotMat[2][1], rotMat[2][2], rotMat[2][3]);
	printf("%.2f, %.2f, %.2f, %.2f\n", rotMat[3][0], rotMat[3][1], rotMat[3][2], rotMat[3][3]);
	printf("----------------------\n");

	vec3 dir;
  //glm_euler_angles(rotMat, dir);
	getEulerAngles(itemQuat, dir);
	item->dir[0] = glm_deg(dir[0]);
	item->dir[1] = glm_deg(dir[1]);
	item->dir[2] = glm_deg(dir[2]);

	glm_mat4_mul(transform, rotMat, transform);

	memcpy(item->transform, transform, sizeof(mat4));
}

void updateItemTransformRot(Item* item)
{
	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, item->pos);

	vec3 dir;
	vec4 itemQuat;
	dir[0] = glm_rad(item->dir[0]);
	dir[1] = glm_rad(item->dir[1]);
	dir[2] = glm_rad(item->dir[2]);
	glm_euler_yzx_quat(dir, itemQuat);
	mat4 rotMat;
	glm_quat_mat4(itemQuat, rotMat);

	glm_mat4_mul(transform, rotMat, transform);

	memcpy(item->quat, itemQuat, sizeof(vec4));
	memcpy(item->transform, transform, sizeof(mat4));
}
