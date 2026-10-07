#include "antline/antline.h"
#include "assetManager.h"
#include "cglm/cglm.h"
#include "cjson.h"
#include "dynList.h"
#include "export/entity.h"
#include "export/overlay.h"
#include "item/item.h"
#include "renderer/debug.h"
#include "renderer/renderer.h"
#include "selection.h"
#include "utils.h"
#include "voxel/itemIntersection.h"
#include <stdio.h>
#include <string.h>

Antline* antlines;
static AntlineConfig config;

static char* loadFields(cJSON* json, const char* name)
{
	cJSON* item = cJSON_GetObjectItem(json, name);
	if (item == 0)
		errorf("undefined field %s in antline.json\n", name);
	const char* v = cJSON_GetStringValue(item);
	return strdup(v);
}

static int loadFieldi(cJSON* json, const char* name)
{
	cJSON* item = cJSON_GetObjectItem(json, name);
	if (item == 0)
		errorf("undefined field %s in antline.json\n", name);
	int v = cJSON_GetNumberValue(item);
	return v;
}

static __attribute__((constructor)) void init()
{
	antlines = dynList_new(0, sizeof(Antline));
}

void loadAntlineConfig()
{
	FILE* file = fopen("antline.json", "rb");
	if (file == 0)
		errorf("failed to open antline.json\n");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);
	if (err)
		errorf("failed to parse antline.json\n%s\n", err);

	config.dotSize = loadFieldi(json, "dotSize");
	config.checkSize = loadFieldi(json, "checkSize");

	config.antlineTex = loadFields(json, "lineTex");
	config.antlineCornerTex = loadFields(json, "cornerTex");
	config.antlineCheckTex = loadFields(json, "checkTex");

	config.antlineWidth = loadFieldi(json, "lineDotCountx");
	config.antlineHeight = loadFieldi(json, "lineDotCounty");
	config.antlineCornerWidth = loadFieldi(json, "cornerDotCountx");
	config.antlineCornerHeight = loadFieldi(json, "cornerDotCounty");

	char* antlineTex = loadFields(json, "editorAntlineTex");
	config.editorAntlineTex = assetManagerLoadTexture(antlineTex);
	char* antlineActiveTex = loadFields(json, "editorAntlineActiveTex");
	config.editorAntlineActiveTex = assetManagerLoadTexture(antlineActiveTex);

	char* checkTex = loadFields(json, "editorCheckTex");
	config.editorCheckTex = assetManagerLoadTexture(checkTex);
	char* checkActiveTex = loadFields(json, "editorCheckActiveTex");
	config.editorCheckActiveTex = assetManagerLoadTexture(checkActiveTex);

	free(antlineTex);
	free(antlineActiveTex);
	free(checkTex);
	free(checkActiveTex);

	config.editorCheckSize = (float)config.checkSize / 128.0f;
	config.editorDotSize = (float)config.dotSize / 128.0f;

	cJSON_Delete(json);
}

Antline* addAntline()
{
	int len = dynList_size(antlines);
	dynList_resize((void*)&antlines, len + 1);
	Antline* antline = &antlines[len];
	memset(antline, 0, sizeof(Antline));
	antline->id = len;
	antlineUpdateTransformRot(&antline->baseSegment);
	antline->segments = dynList_new(0, sizeof(AntlineSegment));
	return antline;
}

void removeAntline(Antline* antline)
{
	dynList_free(antline->segments);

	Item* itemList = getItemList();
	int itemCount = dynList_size(itemList);
	for (int i = 0; i < itemCount; i++)
	{
		Item* item2 = &itemList[i];
		if (!isItemValid(item2))
			continue;
		ItemOutput* outputs = item2->outputs;
		int outputCount = dynList_size(outputs);
		for (int j = 0; j < outputCount; j++)
		{
			ItemOutput* output = &outputs[j];
			if (output->antline == antline->id)
				output->antline = -1;
		}
	}
	antline->id = -1;
	if (selection.type == SELECTION_ANTLINE)
	{
		if (selection.antline == antline)
			selection.type = SELECTION_NONE;
	}
}
char isAntlineValid(Antline* antline)
{
	return antline->id != -1;
}

void antlineUpdateTransform(AntlineSegment* antline)
{
	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, antline->pos);

	mat4 rotMat;
	vec4 itemQuat;
	memcpy(itemQuat, antline->quat, sizeof(vec4));
	glm_quat_mat4(itemQuat, rotMat);

	vec3 dir;
	getEulerAngles(rotMat, dir);
	antline->rot[0] = glm_deg(dir[0]);
	antline->rot[1] = glm_deg(dir[1]);
	antline->rot[2] = glm_deg(dir[2]);

	glm_mat4_mul(transform, rotMat, transform);
	mat4 invTransform;
	glm_mat4_inv_fast(transform, invTransform);

	memcpy(antline->transform, transform, sizeof(mat4));
	memcpy(antline->invTransform, invTransform, sizeof(mat4));
}

void antlineUpdateTransformRot(AntlineSegment* antline)
{
	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, antline->pos);

	vec3 dir;
	vec4 itemQuat;
	dir[0] = glm_rad(antline->rot[0]);
	dir[1] = glm_rad(antline->rot[1]);
	dir[2] = glm_rad(antline->rot[2]);
	glm_euler_yzx_quat(dir, itemQuat);
	mat4 rotMat;
	glm_quat_mat4(itemQuat, rotMat);

	glm_mat4_mul(transform, rotMat, transform);
	mat4 invTransform;
	glm_mat4_inv_fast(transform, invTransform);

	memcpy(antline->quat, itemQuat, sizeof(vec4));
	memcpy(antline->transform, transform, sizeof(mat4));
	memcpy(antline->invTransform, invTransform, sizeof(mat4));
}

static void drawAntline(Antline* antline)
{
	if (antline->hasCheck)
	{
		int tex = 0;
		if (antline->hovered)
			tex = config.editorCheckActiveTex;
		else
			tex = config.editorCheckTex;
		vec3 start = {-config.editorCheckSize, -config.editorCheckSize, -0.125f};
		vec3 end = {config.editorCheckSize, config.editorCheckSize, 0.125f};
		overlayDrawRect(start, end, tex, 1);
		panelEndFrame(antline->baseSegment.transform, antline->hovered == 0, antline->hovered == 0);
	}

	int tex = 0;
	if (antline->hovered)
		tex = config.editorAntlineActiveTex;
	else
		tex = config.editorAntlineTex;
	int l = dynList_size(antline->segments);
	for (int j = 0; j < l; j++)
	{
		AntlineSegment* segment = &antline->segments[j];

		vec3 start = {-config.editorDotSize, -config.editorDotSize, -0.125f};
		vec3 end = {config.editorDotSize, config.editorDotSize, 0.125f};
		end[0] *= 2.0f * segment->len - 1;
		overlayDrawRect(start, end, tex, segment->len);
		panelEndFrame(segment->transform, antline->hovered == 0, antline->hovered == 0);
	}
}

void renderAntlines()
{
	int len = dynList_size(antlines);
	for (int i = 0; i < len; i++)
	{
		Antline* antline = &antlines[i];
		if (!isAntlineValid(antline))
			continue;
		if (antline->hovered)
			continue;

		drawAntline(antline);
	}
}

void renderAntlineHover()
{
	int len = dynList_size(antlines);
	for (int i = 0; i < len; i++)
	{
		Antline* antline = &antlines[i];
		if (!isAntlineValid(antline))
			continue;
		if (antline->hovered == 0)
			continue;
		drawAntline(antline);
	}
	if (selection.type == SELECTION_ANTLINE)
	{

		if (&selection.antline->baseSegment == selection.antlineSeg)
		{
			AntlineSegment* segment = &selection.antline->baseSegment;
			vec3 start = {-config.editorCheckSize, -config.editorCheckSize, -0.125f};
			vec3 end = {config.editorCheckSize, config.editorCheckSize, 0.125f};
			drawDebugRectAntline(start, end, segment->transform);
		}
		else
		{
			AntlineSegment* segment = selection.antlineSeg;
			vec3 start = {-config.editorDotSize, -config.editorDotSize, -0.125f};
			vec3 end = {config.editorDotSize, config.editorDotSize, 0.125f};
			end[0] *= 2.0f * segment->len - 1;
			drawDebugRectAntline(start, end, segment->transform);
		}
	}
}

Antline* getIntersectingAntline(vec3 pos, AntlineSegment** seg)
{
	int count = dynList_size(antlines);
	for (int j = 0; j < count; j++)
	{
		Antline* antline = &antlines[j];
		if (!isAntlineValid(antline))
			continue;
		vec4 pos2 = {pos[0], pos[1], pos[2], 1};
		mat4 transform;
		memcpy(transform, antline->baseSegment.invTransform, sizeof(mat4));
		glm_mat4_mulv(transform, pos2, pos2);
		vec3 bound1 = {-config.editorCheckSize, -config.editorCheckSize, -0.125f};
		vec3 bound2 = {config.editorCheckSize, config.editorCheckSize, 0.125f};

		char hit = 1;
		if (pos2[0] < bound1[0] || pos2[0] > bound2[0])
			hit = 0;
		if (pos2[1] < bound1[1] || pos2[1] > bound2[1])
			hit = 0;
		if (pos2[2] < bound1[2] || pos2[2] > bound2[2])
			hit = 0;
		if (hit)
		{
			*seg = &antline->baseSegment;
			return antline;
		}

		for (int k = 0; k < dynList_size(antline->segments); k++)
		{
			AntlineSegment* segment = &antline->segments[k];
			vec4 pos2 = {pos[0], pos[1], pos[2], 1};
			mat4 transform;
			memcpy(transform, segment->invTransform, sizeof(mat4));
			glm_mat4_mulv(transform, pos2, pos2);
			vec3 bound1 = {-config.editorDotSize, -config.editorDotSize, -0.125f};
			vec3 bound2 = {config.editorDotSize, config.editorDotSize, 0.125f};
			bound2[0] *= 2.0f * segment->len - 1;

			if (pos2[0] < bound1[0] || pos2[0] > bound2[0])
				continue;
			if (pos2[1] < bound1[1] || pos2[1] > bound2[1])
				continue;
			if (pos2[2] < bound1[2] || pos2[2] > bound2[2])
				continue;
			*seg = segment;
			return antline;
		}
	}
	return 0;
}

void antlineExport()
{
	int count = dynList_size(antlines);
	for (int i = 0; i < count; i++)
	{
		Antline* antline = &antlines[i];
		if (!isAntlineValid(antline))
			continue;
		int segmentCount = dynList_size(antline->segments);
		if (segmentCount == 0 && !antline->hasCheck)
			continue;

		char buf[64];
		snprintf(buf, 64, "antline%d", antline->id);

		if (antline->hasCheck)
		{
			Overlay* overlay = exportCreateOverlay();
			overlay->name = strdup(buf);
			memcpy(overlay->pos, antline->baseSegment.pos, sizeof(vec3));
			memcpy(overlay->rotation, antline->baseSegment.rot, sizeof(vec3));
			overlay->tile[0] = 1;
			overlay->tile[1] = 1;
			overlay->texture = config.antlineCheckTex;
			overlay->size[0] = config.checkSize;
			overlay->size[1] = config.checkSize;
		}
		for (int i = 0; i < segmentCount; i++)
		{
			char addCorner = 1;
			AntlineSegment* segment = &antline->segments[i];

			vec3 start = {-config.editorDotSize, -config.editorDotSize, -0.125f};
			vec3 end = {config.editorDotSize, config.editorDotSize, 0.125f};
			OBB a;
			start[0] += 0.01;
			start[1] += 0.01;
			end[0] -= 0.01;
			end[1] -= 0.01;

			vec3 rot;
			memcpy(rot, segment->rot, sizeof(vec3));
			rot[0] = glm_rad(rot[0]);
			rot[1] = glm_rad(rot[1]);
			rot[2] = glm_rad(rot[2]);
			mat4 rotMat;
			glm_euler_yzx(rot, rotMat);
			genOBB(start, end, rotMat, segment->pos, &a);

			for (int j = 0; j < segmentCount; j++)
			{
				if (i == j)
					continue;
				AntlineSegment* segment2 = &antline->segments[j];
				vec3 start = {-config.editorDotSize, -config.editorDotSize, -0.125f};
				vec3 end = {config.editorDotSize, config.editorDotSize, 0.125f};
				end[0] *= 2.0f * segment2->len - 1;
				start[0] += 0.01;
				start[1] += 0.01;
				end[0] -= 0.01;
				end[1] -= 0.01;
				OBB b;

				vec3 rot;
				memcpy(rot, segment2->rot, sizeof(vec3));
				rot[0] = glm_rad(rot[0]);
				rot[1] = glm_rad(rot[1]);
				rot[2] = glm_rad(rot[2]);
				mat4 rotMat;
				glm_euler_yzx(rot, rotMat);
				genOBB(start, end, rotMat, segment2->pos, &b);
				if (getCollision(&a, &b))
				{
					printf("seg %d, %d collided\n", i, j);
					addCorner = 0;
				}
			}

			if (!(addCorner && segment->len == 1))
			{
				Overlay* overlay = exportCreateOverlay();
				vec3 bound1 = {-config.editorDotSize, -config.editorDotSize, 0};
				vec3 bound2 = {config.editorDotSize, config.editorDotSize, 0};
				bound2[0] *= 2.0f * segment->len - 1;
				if (addCorner)
					bound1[0] = config.editorDotSize;

				float x = (bound1[0] + bound2[0]) / 2.0f;

				vec4 pos = {x, 0, 0, 1};
				mat4 transform;
				memcpy(transform, segment->transform, sizeof(mat4));
				glm_mat4_mulv(transform, pos, pos);

				memcpy(overlay->pos, pos, sizeof(vec3));

				overlay->name = strdup(buf);
				memcpy(overlay->rotation, segment->rot, sizeof(vec3));
				overlay->tile[0] = (segment->len - addCorner) / (float)config.antlineHeight;
				overlay->tile[1] = 1.0f / (float)config.antlineWidth;
				overlay->texture = config.antlineTex;
				overlay->size[0] = config.dotSize;
				overlay->size[1] = config.dotSize * (segment->len - addCorner);
			}
			if (addCorner)
			{
				Overlay* overlay = exportCreateOverlay();
				vec3 bound1 = {-config.editorDotSize, -config.editorDotSize, 0};
				vec3 bound2 = {config.editorDotSize, config.editorDotSize, 0};

				memcpy(overlay->pos, segment->pos, sizeof(vec3));

				overlay->name = strdup(buf);
				memcpy(overlay->rotation, segment->rot, sizeof(vec3));
				overlay->tile[0] = 1.0f / (float)config.antlineCornerHeight;
				overlay->tile[1] = 1.0f / (float)config.antlineCornerWidth;
				overlay->texture = config.antlineCornerTex;
				overlay->size[0] = config.dotSize;
				overlay->size[1] = config.dotSize;
			}
		}
		Entity* entity = exportCreateEntity();

		memcpy(entity->pos, antline->baseSegment.pos, sizeof(vec3));
		memcpy(entity->rotation, antline->baseSegment.rot, sizeof(vec3));
		entity->className = "env_texturetoggle";
		exportEntityAddKvss(entity, "target", buf);
		snprintf(buf, 64, "antline%d-tex", antline->id);
		entity->name = strdup(buf);
	}
}
