#include "item.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "cglm/util.h"
#include "item/entityItem.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "jsonUtils.h"
#include "raycast.h"
#include "renderer/debug.h"
#include "utils.h"

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
		vec4 pos2 = {pos[0], pos[1], pos[2], 1};

		mat4 transform;
		memcpy(transform, item->invTransform, sizeof(mat4));
		glm_mat4_mulv(transform, pos2, pos2);
		vec4* bound1 = &item->def->bound1;
		vec4* bound2 = &item->def->bound2;
		if (pos2[0] < (*bound1)[0] || pos2[0] > (*bound2)[0])
			continue;
		if (pos2[1] < (*bound1)[1] || pos2[1] > (*bound2)[1])
			continue;
		if (pos2[2] < (*bound1)[2] || pos2[2] > (*bound2)[2])
			continue;
		return item;
	}
	return 0;
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
	getEulerAngles(rotMat, dir);
	item->dir[0] = glm_deg(dir[0]);
	item->dir[1] = glm_deg(dir[1]);
	item->dir[2] = glm_deg(dir[2]);

	glm_mat4_mul(transform, rotMat, transform);
	mat4 invTransform;
	glm_mat4_inv_fast(transform, invTransform);

	memcpy(item->transform, transform, sizeof(mat4));
	memcpy(item->invTransform, invTransform, sizeof(mat4));
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
	mat4 invTransform;
	glm_mat4_inv_fast(transform, invTransform);

	memcpy(item->quat, itemQuat, sizeof(vec4));
	memcpy(item->transform, transform, sizeof(mat4));
	memcpy(item->invTransform, invTransform, sizeof(mat4));
}
