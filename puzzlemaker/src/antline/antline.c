#include "antline/antline.h"
#include "assetManager.h"
#include "cglm/cglm.h"
#include "cjson.h"
#include "dynList.h"
#include "export/entity.h"
#include "export/overlay.h"
#include "renderer/debug.h"
#include "renderer/renderer.h"
#include "selection.h"
#include "utils.h"
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

	config.antlineLen = loadFieldi(json, "lineDotCountx");
	config.antlineWidth = loadFieldi(json, "lineDotCounty");
	config.antlineCornerLen = loadFieldi(json, "cornerDotCountx");
	config.antlineCornerWidth = loadFieldi(json, "cornerDotCounty");

	char* antlineTex = loadFields(json, "editorAntlineTex");
	config.editorAntlineTex = assetManagerLoadTexture(antlineTex);

	char* checkTex = loadFields(json, "editorCheckTex");
	config.editorCheckTex = assetManagerLoadTexture(checkTex);

	free(antlineTex);
	free(checkTex);

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
	antline->segments = dynList_new(0, sizeof(AntlineSegment));
	return antline;
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

void renderAntlines()
{
	int len = dynList_size(antlines);
	for (int i = 0; i < len; i++)
	{
		Antline* antline = &antlines[i];
		if (antline->hasCheck)
		{
			vec3 start = {-config.editorCheckSize, -config.editorCheckSize, -0.125f};
			vec3 end = {config.editorCheckSize, config.editorCheckSize, 0.125f};
			overlayDrawRect(start, end, config.editorCheckTex);
			panelEndFrame(antline->baseSegment.transform, 1);
			if (selection.type == SELECTION_ANTLINE && antline == selection.antline)
				drawDebugRectAntline(start, end, antline->baseSegment.transform);
		}

		int l = dynList_size(antline->segments);
		for (int j = 0; j < l; j++)
		{
			AntlineSegment* segment = &antline->segments[j];

			vec3 start = {-config.editorDotSize, -config.editorDotSize, -0.125f};
			vec3 end = {config.editorDotSize, config.editorDotSize, 0.125f};
			end[0] *= 2.0f * segment->len;
			overlayDrawRect(start, end, config.editorAntlineTex);
			panelEndFrame(segment->transform, 1);
			if (selection.type == SELECTION_ANTLINE_SEG && segment == selection.antlineSeg)
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
		if(hit)
			return antline;

		for (int k = 0; k < dynList_size(antline->segments); k++)
		{
			AntlineSegment* segment = &antline->segments[k];
			vec4 pos2 = {pos[0], pos[1], pos[2], 1};
			mat4 transform;
			memcpy(transform, antline->baseSegment.invTransform, sizeof(mat4));
			glm_mat4_mulv(transform, pos2, pos2);
			vec3 bound1 = {-config.editorCheckSize, -config.editorCheckSize, -0.125f};
			vec3 bound2 = {config.editorCheckSize, config.editorCheckSize, 0.125f};

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

		if (antline->hasCheck)
		{
			Overlay* overlay = exportCreateOverlay();
			char buf[64];
			snprintf(buf, 64, "antline%d", antline->id);
			overlay->name = strdup(buf);
			memcpy(overlay->pos, antline->baseSegment.pos, sizeof(vec3));
			memcpy(overlay->rotation, antline->baseSegment.rot, sizeof(vec3));
			overlay->tile[0] = 1;
			overlay->tile[1] = 1;
			overlay->texture = config.antlineCheckTex;
			overlay->size[0] = config.checkSize;
			overlay->size[1] = config.checkSize;
			overlay->tint[0] = 255;
			overlay->tint[1] = 255;
			overlay->tint[2] = 255;
			overlay->tint[3] = 255;

			Entity* entity = exportCreateEntity();
			memcpy(entity->pos, antline->baseSegment.pos, sizeof(vec3));
			memcpy(entity->rotation, antline->baseSegment.rot, sizeof(vec3));
			entity->className = "env_texturetoggle";
			exportEntityAddKvss(entity, "target", buf);
			snprintf(buf, 64, "antline%d-tex", antline->id);
			entity->name = strdup(buf);
		}
	}
}
