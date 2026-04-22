#include "item.h"
#include "camera.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include "jsonUtils.h"
#include "renderer/debug.h"
#include "renderer/renderer.h"

#include <cjson.h>
#include <dynList.h>
#include <math.h>
#include <string.h>

ItemDefinition* definitions;
Item* items;

Item* selectedItem;

char moving = 0;

void drawItems()
{
	int len = dynList_size(items);
	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		if (item->index == -1)
			continue;
		drawMesh(&item->def->mesh, item->def->texture, item->transform);
		// drawDebugRect(item->def->bound1, item->def->bound2);
	}
}

void updateSelectedItem();

Item* findSelectedItem(vec3 start, vec3 dir, float len)
{
	// vec3 dir;
	// glm_normalize_to(dir2, dir);
	vec3 end = {start[0] + dir[0] * len, start[1] + dir[1] * len, start[2] + dir[2] * len};

	int count = dynList_size(items);

	for (float i = 0; i < 1.0f; i += 0.01f)
	{
		vec3 pv;
		glm_vec3_lerp(start, end, i, pv);

		for (int j = 0; j < count; j++)
		{
			Item* item = &items[j];
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
			glm_vec3_sub(pv, p1, v);

			float a = glm_vec3_dot(v, i);
			float b = glm_vec3_dot(i, i);
			float c = glm_vec3_dot(v, j);
			float d = glm_vec3_dot(j, j);
			float e = glm_vec3_dot(v, k);
			float f = glm_vec3_dot(k, k);

			if (0 < a && a < b && 0 < c && c < d && 0 < e && e < f)
			{
				return item;
			}
		}
	}
	return 0;
}

Item* getSelectedItem()
{
	return selectedItem;
}

void setSelectedItem(Item* item)
{
	selectedItem = item;
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

ItemDefinition* getItemDefinitions()
{
	return definitions;
}

Item** getItems()
{
	return &items;
}

Item* addItem(int id)
{
	int len = dynList_size(items);
	int index = -1;
	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		if (item->index == -1)
		{
			index = i;
			break;
		}
	}
	if (index == -1)
	{
		dynList_resize((void**)&items, len + 1);
		index = len;
	}
	Item* item = &items[index];
	item->index = index;
	item->id = id;

	vec3 offset;
	glm_vec3_scale(forward, 5, offset);

	item->dir[0] = 0;
	item->dir[1] = 0;
	item->dir[2] = 0;

	item->pos[0] = offset[0] + cameraPos[0];
	item->pos[1] = offset[1] + cameraPos[1];
	item->pos[2] = offset[2] + cameraPos[2];

	item->pos[0] = floorf(item->pos[0]);
	item->pos[1] = floorf(item->pos[1]);
	item->pos[2] = floorf(item->pos[2]);

	updateItemTransform(item);

	ItemDefinition* def = &definitions[id];
	item->def = def;

	item->outputs = dynList_new(0, sizeof(ItemOutput));
	int l = dynList_size(def->kvs);
	item->kv = dynList_new(l, sizeof(ItemKv));
	for (int i = 0; i < l; i++)
	{
		item->kv[i].def = &def->kvs[i];
		item->kv[i].value = def->kvs[i].defaultValue;
	}

	selectedItem = item;
	return item;
}

void removeItem(Item* item)
{
	if (item == selectedItem)
		selectedItem = 0;
	dynList_free(item->kv);
	dynList_free(item->outputs);

	item->id = -1;
	item->index = -1;
}

Item* getItem(int i)
{
	return &items[i];
}
