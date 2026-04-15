#include "item.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include "jsonUtils.h"
#include "renderer/debug.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "utils.h"

#include <cjson.h>
#include <dynList.h>
#include <stdio.h>
#include <string.h>

ItemDefinition* definitions;
Item* items;

Item* selectedItem;

char moving = 0;

void initItems()
{
	definitions = dynList_new(0, sizeof(ItemDefinition));
	items = dynList_new(0, sizeof(Item));

	FILE* file = fopen("items.json", "rb");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);

	cJSON* item;
	cJSON_ArrayForEach(item, json)
	{
		int len = dynList_size(definitions);
		dynList_resize((void**)&definitions, len + 1);
		ItemDefinition* def = &definitions[len];

		def->name = jsonGetStr(item, "name");
		printf("name: %s\n", def->name);
		def->modelName = jsonGetStr(item, "model");
		def->textureName = jsonGetStr(item, "mat");
		if (cJSON_HasObjectItem(item, "instance"))
		{
			def->type = ENTITY_TYPE_INSTANCE;
			def->instanceName = jsonGetStr(item, "instance");
		}
		else
		{
			def->type = ENTITY_TYPE_ENTITY;
			def->instanceName = jsonGetStr(item, "entity");
		}

		cJSON* keyValues = cJSON_GetObjectItem(item, "keyvalues");
		len = cJSON_GetArraySize(keyValues);
		def->kvs = dynList_new(len, sizeof(ItemKvDef));

		for (int i = 0; i < len; i++)
		{
			cJSON* kv = cJSON_GetArrayItem(keyValues, i);
			ItemKvDef* kvDef = &def->kvs[i];

			kvDef->name = jsonGetStr(kv, "name");

			cJSON* type = cJSON_GetObjectItem(kv, "type");
			if (strcmp(type->valuestring, "bool") == 0)
			{
				kvDef->type = TYPE_BOOL;
				kvDef->defaultValue.b = jsonGetBool(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "int") == 0)
			{
				kvDef->type = TYPE_INT;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
			}
			if (strcmp(type->valuestring, "drop-string") == 0)
			{
				kvDef->type = TYPE_STRING | TYPE_DROPDOWN;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
				cJSON* options = cJSON_GetObjectItem(kv, "options");
				cJSON* opt = options->child;
				int len = cJSON_GetArraySize(options);

				kvDef->dropNames = dynList_new(len, sizeof(char*));
				kvDef->dropValues = dynList_new(len, sizeof(V));
				int i = 0;
				while (1)
				{
					if (opt == 0)
						break;
					kvDef->dropNames[i] = copyString(opt->string);
					kvDef->dropValues[i].s = copyString(opt->valuestring);
					opt = opt->next;
					i++;
				}
			}
			if (strcmp(type->valuestring, "drop-int") == 0)
			{
				kvDef->type = TYPE_INT | TYPE_DROPDOWN;
				kvDef->defaultValue.i = jsonGetInt(kv, "defaultValue");
				cJSON* options = cJSON_GetObjectItem(kv, "options");
				cJSON* opt = options->child;
				int len = cJSON_GetArraySize(options);

				kvDef->dropNames = dynList_new(len, sizeof(char*));
				kvDef->dropValues = dynList_new(len, sizeof(V));
				int i = 0;
				while (1)
				{
					if (opt == 0)
						break;
					kvDef->dropNames[i] = copyString(opt->string);
					kvDef->dropValues[i].i = opt->valuedouble;
					opt = opt->next;
					i++;
				}
			}
		}

		cJSON* inputs = cJSON_GetObjectItem(item, "inputs");
		len = cJSON_GetArraySize(inputs);
		def->inputs = dynList_new(len, sizeof(InputDef));
		for (int i = 0; i < len; i++)
		{
			cJSON* input = cJSON_GetArrayItem(inputs, i);
			InputDef* inputDef = &def->inputs[i];

			inputDef->name = jsonGetStr(input, "name");
			inputDef->trueInput = jsonGetStr(input, "trueInput");
			inputDef->falseInput = jsonGetStr(input, "falseInput");
		}

		cJSON* outputs = cJSON_GetObjectItem(item, "outputs");
		len = cJSON_GetArraySize(outputs);
		def->outputs = dynList_new(len, sizeof(OutputDef));
		for (int i = 0; i < len; i++)
		{
			cJSON* output = cJSON_GetArrayItem(outputs, i);
			OutputDef* outputDef = &def->outputs[i];

			outputDef->name = jsonGetStr(output, "name");
			outputDef->trueOutput = jsonGetStr(output, "trueOutput");
			outputDef->falseOutput = jsonGetStr(output, "falseOutput");
		}

		cJSON* bound = cJSON_GetObjectItem(item, "bound1");
		def->bound1[0] = jsonArrGetFloat(bound, 0);
		def->bound1[1] = jsonArrGetFloat(bound, 1);
		def->bound1[2] = jsonArrGetFloat(bound, 2);
		def->bound1[3] = 1;

		bound = cJSON_GetObjectItem(item, "bound2");
		def->bound2[0] = jsonArrGetFloat(bound, 0);
		def->bound2[1] = jsonArrGetFloat(bound, 1);
		def->bound2[2] = jsonArrGetFloat(bound, 2);
		def->bound2[3] = 1;

		def->mesh = loadMesh(def->modelName);
		def->texture = loadTexture(def->textureName);

		cJSON* staticKvs = cJSON_GetObjectItem(item, "statickvs");
		len = cJSON_GetArraySize(staticKvs);
		def->staticKvs = dynList_new(len, sizeof(char*));
		for (int i = 0; i < len; i++)
		{
			char* s = (char*)jsonArrGetStr(staticKvs, i);
			for (int j = 0; j < strlen(s); j++)
			{
				if (s[j] == '\'')
					s[j] = '\"';
			}
			def->staticKvs[i] = s;
		}
		cJSON* offset = cJSON_GetObjectItem(item, "bound2");
		if (offset)
		{
			def->offset[0] = jsonArrGetFloat(offset, 0);
			def->offset[1] = jsonArrGetFloat(offset, 1);
			def->offset[2] = jsonArrGetFloat(offset, 2);
		}
		else
		{
			def->offset[0] = 0;
			def->offset[1] = 0;
			def->offset[2] = 0;
		}
	}

	err = cJSON_GetErrorPtr();
	if (err)
	{
		printf("json error\n%s\n", err);
	}
	cJSON_free(json);
}

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

	item->dir[0] = 0;
	item->dir[1] = 0;
	item->dir[2] = 0;

	item->pos[0] = 0;
	item->pos[1] = 0;
	item->pos[2] = 0;

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
