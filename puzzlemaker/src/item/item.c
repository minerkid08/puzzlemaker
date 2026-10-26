#include "item/item.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "cglm/util.h"
#include "jsonUtils.h"
#include "raycast.h"
#include "renderer/debug.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "selection.h"
#include "settings.h"
#include "utils.h"

#include <cjson.h>
#include <dynList.h>
#include <glad/glad.h>
#include <string.h>

extern Item* itemList;

char moving = 0;

char isItemValid(Item* item)
{
	return item->index != -1;
}

void drawItems()
{
	Item* highlightedItem = 0;
	int len = dynList_size(itemList);
	for (int i = 0; i < len; i++)
	{
		Item* item = &itemList[i];
		if (!isItemValid(item))
			continue;
		if (!(item->def->flags & TEX_OPAQUE))
			continue;
		if (item->highlighted)
			highlightedItem = item;
		if (item->hidden)
			continue;
		item->def->callbacks->render(item);
	}

	setRenderPass(1);
	glDepthMask(GL_FALSE);
	for (int i = 0; i < len; i++)
	{
		Item* item = &itemList[i];
		if (!isItemValid(item))
			continue;
		if (!(item->def->flags & TEX_TRANSPARENT))
			continue;
		if (item->hidden)
			continue;
		item->def->callbacks->render(item);
	}
	setRenderPass(0);
	glDepthMask(GL_TRUE);

	if (highlightedItem)
	{
		vec3 a;
		vec3 b;
		highlightedItem->def->callbacks->getBoundingBox(highlightedItem, a, b);
		drawDebugRect(a, b, highlightedItem->transform, 1);
		highlightedItem->highlighted = 0;
	}

	if (selection.type == SELECTION_ITEM)
	{
		vec3 a;
		vec3 b;
		Item* item = selection.item;
		item->def->callbacks->getBoundingBox(item, a, b);
		drawDebugRect(a, b, item->transform, 0);

		int c = 1;
		for (int i = 0; i < dynList_size(item->def->kvs); i++)
		{
			ItemKv* kv = &item->kv[i];
			ItemKvDef* def = kv->def;
			int type = def->type & ~(TYPE_INSTANCE);
			if (type != TYPE_VEC3)
				continue;
			if (def->dropValues->i == GIZMO_NONE)
				continue;
			vec3 v;
			memcpy(v, kv->value.v, sizeof(vec3));
			if (def->dropValues->i == GIZMO_POS)
			{
				v[0] -= item->pos[0];
				v[1] -= item->pos[1];
				v[2] -= item->pos[2];
			}
			if (def->dropValues->i == GIZMO_ANGLE || def->dropValues->i == GIZMO_ANGLE_OFFSET)
			{
				v[0] = glm_rad(v[0]);
				v[1] = glm_rad(v[1]);
				v[2] = glm_rad(v[2]);
				vec4 quat;
				glm_euler_yxz_quat(v, quat);

				if (def->dropValues->i == GIZMO_ANGLE_OFFSET)
				{
					vec4 quat2;
					memcpy(quat2, item->quat, sizeof(vec4));
					glm_quat_mul(quat2, quat, quat);
				}

				v[0] = 0;
				v[1] = 0;
				v[2] = editorSettings.gizmoLen;
				mat4 m;
				glm_quat_mat4(quat, m);
				glm_mat4_mulv3(m, v, 1, v);
			}
			drawDebugPoint(v, item->pos, c++);
		}
	}
}

Item** getIntersectingItems(vec3 pos, Item** ignore)
{
	Item** outItems = dynList_new(0, sizeof(Item*));
	int outItemsCount = 0;
	int count = dynList_size(itemList);
	int ignoreCount = 0;
	if (ignore)
		ignoreCount = dynList_size(ignore);

	for (int j = 0; j < count; j++)
	{
		Item* item = &itemList[j];
		if (!isItemValid(item))
			continue;
		if (ignore)
		{
			char skip = 0;
			for (int k = 0; k < ignoreCount; k++)
			{
				if (item == ignore[k])
				{
					skip = 1;
					break;
				}
			}
			if (skip)
				continue;
		}
		vec4 pos2 = {pos[0], pos[1], pos[2], 1};
		if (item->def == 0)
			continue;
		mat4 transform;
		memcpy(transform, item->invTransform, sizeof(mat4));
		glm_mat4_mulv(transform, pos2, pos2);
		vec3 bound1;
		vec3 bound2;

		item->def->callbacks->getBoundingBox(item, bound1, bound2);

		if (pos2[0] < bound1[0] || pos2[0] > bound2[0])
			continue;
		if (pos2[1] < bound1[1] || pos2[1] > bound2[1])
			continue;
		if (pos2[2] < bound1[2] || pos2[2] > bound2[2])
			continue;
		dynList_resize((void*)&outItems, outItemsCount + 1);
		outItems[outItemsCount] = item;
		outItemsCount++;
	}
	return outItems;
}

Item* getIntersectingItem(vec3 pos, Item** ignore)
{
	int count = dynList_size(itemList);
	int ignoreCount = 0;
	if (ignore)
		ignoreCount = dynList_size(ignore);

	for (int j = 0; j < count; j++)
	{
		Item* item = &itemList[j];
		if (!isItemValid(item))
			continue;
		if (item->hidden)
			continue;
		if (ignore)
		{
			char skip = 0;
			for (int k = 0; k < ignoreCount; k++)
			{
				if (item == ignore[k])
				{
					skip = 1;
					break;
				}
			}
			if (skip)
				continue;
		}
		vec4 pos2 = {pos[0], pos[1], pos[2], 1};
		if (item->def == 0)
			continue;
		mat4 transform;
		memcpy(transform, item->invTransform, sizeof(mat4));
		glm_mat4_mulv(transform, pos2, pos2);
		vec3 bound1;
		vec3 bound2;

		item->def->callbacks->getBoundingBox(item, bound1, bound2);

		if (pos2[0] < bound1[0] || pos2[0] > bound2[0])
			continue;
		if (pos2[1] < bound1[1] || pos2[1] > bound2[1])
			continue;
		if (pos2[2] < bound1[2] || pos2[2] > bound2[2])
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
	glm_euler_yxz_quat(dir, itemQuat);
	mat4 rotMat;
	glm_quat_mat4(itemQuat, rotMat);

	glm_mat4_mul(transform, rotMat, transform);
	mat4 invTransform;
	glm_mat4_inv_fast(transform, invTransform);

	memcpy(item->quat, itemQuat, sizeof(vec4));
	memcpy(item->transform, transform, sizeof(mat4));
	memcpy(item->invTransform, invTransform, sizeof(mat4));
}
