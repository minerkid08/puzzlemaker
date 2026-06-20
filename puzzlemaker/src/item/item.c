#include "item.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include "item/entityItem.h"
#include "item/volumeItem.h"
#include "item/panel.h"
#include "jsonUtils.h"
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
    if(item->def->type == ITEM_TYPE_ENTITY)
      entityItemRender(item);
    if(item->def->type == ITEM_TYPE_PANEL)
      panelItemRender(item);
    if(item->def->type == ITEM_TYPE_VOLUME)
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

void updateItemTransform(Item* item)
{
	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, item->pos);

	glm_rotate_z(transform, glm_rad(item->dir[2]), transform);
	glm_rotate_y(transform, glm_rad(item->dir[1]), transform);
	glm_rotate_x(transform, glm_rad(item->dir[0]), transform);
	memcpy(item->transform, transform, sizeof(mat4));
}
