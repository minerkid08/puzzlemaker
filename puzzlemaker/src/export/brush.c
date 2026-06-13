#include "brush.h"
#include "cglm/vec3.h"
#include "dynList.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static Brush* brushes;

void exportStartBrushes()
{
	brushes = dynList_new(0, sizeof(Brush));
	dynList_reserve((void**)&brushes, 128);
}

Brush* exportCreateBrush(vec3 start, vec3 end)
{
	float minx = min(start[0], end[0]);
	float miny = min(start[1], end[1]);
	float minz = min(start[2], end[2]);

	float maxx = max(start[0], end[0]);
	float maxy = max(start[1], end[1]);
	float maxz = max(start[2], end[2]);

	vec3 vertxyz = {minx, miny, minz};
	vec3 vertXyz = {maxx, miny, minz};
	vec3 vertxYz = {minx, maxy, minz};
	vec3 vertXYz = {maxx, maxy, minz};
	vec3 vertxyZ = {minx, miny, maxz};
	vec3 vertXyZ = {maxx, miny, maxz};
	vec3 vertxYZ = {minx, maxy, maxz};
	vec3 vertXYZ = {maxx, maxy, maxz};

	vec2 uvxy = {0, 0};
	vec2 uvXy = {1, 0};
	vec2 uvxY = {0, 1};
	vec2 uvXY = {1, 1};

	int len = dynList_size(brushes);
	dynList_resize((void**)&brushes, len + 1);
	Brush* brush = &brushes[len];
	brush->id = len + 1;
	brush->ent = 0;

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];
		side->id = i;
		side->lightmapscale = 16;
		side->fit = 0;
		memcpy(side->uvs[0], uvxy, sizeof(vec2));
		memcpy(side->uvs[1], uvXy, sizeof(vec2));
		memcpy(side->uvs[2], uvxY, sizeof(vec2));
		memcpy(side->uvs[3], uvXY, sizeof(vec2));
	}

	Side* side = &brush->sides[DIR_POS_X];
	memcpy(side->verts[0], vertXyz, sizeof(vec3));
	memcpy(side->verts[1], vertXyZ, sizeof(vec3));
	memcpy(side->verts[2], vertXYZ, sizeof(vec3));
	memcpy(side->verts[3], vertXYz, sizeof(vec3));

	side = &brush->sides[DIR_NEG_X];
	memcpy(side->verts[0], vertxyZ, sizeof(vec3));
	memcpy(side->verts[1], vertxyz, sizeof(vec3));
	memcpy(side->verts[2], vertxYz, sizeof(vec3));
	memcpy(side->verts[3], vertxYZ, sizeof(vec3));

	side = &brush->sides[DIR_POS_Y];
	memcpy(side->verts[0], vertxYz, sizeof(vec3));
	memcpy(side->verts[1], vertXYz, sizeof(vec3));
	memcpy(side->verts[2], vertXYZ, sizeof(vec3));
	memcpy(side->verts[3], vertxYZ, sizeof(vec3));

	side = &brush->sides[DIR_NEG_Y];
	memcpy(side->verts[0], vertXyz, sizeof(vec3));
	memcpy(side->verts[1], vertxyz, sizeof(vec3));
	memcpy(side->verts[2], vertxyZ, sizeof(vec3));
	memcpy(side->verts[3], vertXyZ, sizeof(vec3));

	side = &brush->sides[DIR_POS_Z];
	memcpy(side->verts[0], vertXyZ, sizeof(vec3));
	memcpy(side->verts[1], vertxyZ, sizeof(vec3));
	memcpy(side->verts[2], vertxYZ, sizeof(vec3));
	memcpy(side->verts[3], vertXYZ, sizeof(vec3));

	side = &brush->sides[DIR_NEG_Z];
	memcpy(side->verts[0], vertxyz, sizeof(vec3));
	memcpy(side->verts[1], vertXyz, sizeof(vec3));
	memcpy(side->verts[2], vertXYz, sizeof(vec3));
	memcpy(side->verts[3], vertxYz, sizeof(vec3));

	return brush;
}

void planeIntersect(vec3 planePos, vec3 planeNormal, vec3 rayPos, vec3 rayDir, vec3 out)
{
	vec3 norm = {-planeNormal[0], -planeNormal[1], -planeNormal[2]};
	float d = glm_vec3_dot(planePos, norm);
	float t = (-d + glm_vec3_dot(rayPos, planeNormal)) / glm_vec3_dot(rayDir, planeNormal);
	glm_vec3_scale(rayDir, t, norm);
	out[0] = norm[0] + rayPos[0];
	out[1] = norm[1] + rayPos[1];
	out[2] = norm[2] + rayPos[2];
}

void exportBrush(FILE* file, Brush* brush)
{
	fprintf(file, "solid\n{\n");
	fprintf(file, "  \"id\" \"%d\"\n", brush->id);
	for (int j = 0; j < 6; j++)
	{
		Side* side = &brush->sides[j];
		char v1[40];
		char v2[40];
		char v3[40];
		snprintf(v1, 40, "(%.2f %.2f %.2f)", -side->verts[0][0] * 64, side->verts[0][2] * 64, side->verts[0][1] * 64);
		snprintf(v2, 40, "(%.2f %.2f %.2f)", -side->verts[2][0] * 64, side->verts[2][2] * 64, side->verts[2][1] * 64);
		snprintf(v3, 40, "(%.2f %.2f %.2f)", -side->verts[3][0] * 64, side->verts[3][2] * 64, side->verts[3][1] * 64);

		char planeBuf[140];
		snprintf(planeBuf, 140, "%s %s %s", v1, v2, v3);
		fprintf(file, "  side\n  {\n");
		fprintf(file, "    \"id\" \"%d\"\n", side->id + 1);
		fprintf(file, "    \"plane\" \"%s\"\n", planeBuf);

		fprintf(file, "    point_data\n    {\n");
		fprintf(file, "      \"numpts\" \"4\"\n");

		fprintf(file, "      \"point\" \"0 %.2f %.2f %.2f\"\n", -side->verts[0][0] * 64, side->verts[0][2] * 64,
				side->verts[0][1] * 64);
		fprintf(file, "      \"point\" \"1 %.2f %.2f %.2f\"\n", -side->verts[1][0] * 64, side->verts[1][2] * 64,
				side->verts[1][1] * 64);
		fprintf(file, "      \"point\" \"2 %.2f %.2f %.2f\"\n", -side->verts[2][0] * 64, side->verts[2][2] * 64,
				side->verts[2][1] * 64);
		fprintf(file, "      \"point\" \"3 %.2f %.2f %.2f\"\n", -side->verts[3][0] * 64, side->verts[3][2] * 64,
				side->verts[3][1] * 64);

		fprintf(file, "    }\n");

		vec3 uDir;
		vec3 vDir;

		vec3 ref;
		ref[0] = -side->verts[2][0] * 64;
		ref[1] = -side->verts[2][2] * 64;
		ref[2] = -side->verts[2][1] * 64;

		uDir[0] = -side->verts[3][0] * 64 + side->verts[2][0] * 64;
		uDir[1] = side->verts[3][2] * 64 - side->verts[2][2] * 64;
		uDir[2] = side->verts[3][1] * 64 - side->verts[2][1] * 64;

		float horizScale = 0.25;
		if (side->fit)
		{
			float width = sqrtf(uDir[0] * uDir[0] + uDir[1] * uDir[1] + uDir[2] * uDir[2]);
			horizScale = width / side->texWidth;
		}

		vDir[0] = -side->verts[1][0] * 64 + side->verts[2][0] * 64;
		vDir[1] = side->verts[1][2] * 64 - side->verts[2][2] * 64;
		vDir[2] = side->verts[1][1] * 64 - side->verts[2][1] * 64;

		float vertScale = 0.25;
		if (side->fit)
		{
			float height = sqrtf(vDir[0] * vDir[0] + vDir[1] * vDir[1] + vDir[2] * vDir[2]);
			vertScale = height / side->texHeight;
		}
		glm_normalize(uDir);
		glm_normalize(vDir);

		float xoff = 0;
		float yoff = 0;
		if (side->fit)
		{
			vec3 normal;
			glm_cross(uDir, vDir, normal);
			glm_normalize(normal);

			vec3 zero = {0, 0, 0};
			vec3 intersect;
			planeIntersect(ref, normal, zero, normal, intersect);

			normal[0] = -uDir[0];
			normal[1] = -uDir[1];
			normal[2] = -uDir[2];

			vec3 intersect2;
			planeIntersect(ref, normal, intersect, uDir, intersect2);
			vec3 offLine;
			offLine[0] = intersect[0] - intersect2[0];
			offLine[1] = intersect[1] - intersect2[1];
			offLine[2] = intersect[2] - intersect2[2];
			xoff = sqrtf(offLine[0] * offLine[0] + offLine[1] * offLine[1] + offLine[2] * offLine[2]);
			xoff /= horizScale;
			xoff = fmodf(xoff, side->texWidth);
			offLine[0] = ref[0] - intersect2[0];
			offLine[1] = ref[1] - intersect2[1];
			offLine[2] = ref[2] - intersect2[2];
			yoff = sqrtf(offLine[0] * offLine[0] + offLine[1] * offLine[1] + offLine[2] * offLine[2]);
			yoff /= vertScale;
			yoff = fmodf(yoff, side->texHeight);
		}

		fprintf(file, "    \"uaxis\" \"[%.4f %.4f %.4f %.4f] %.4f\"\n", uDir[0], uDir[1], uDir[2], -xoff, horizScale);
		fprintf(file, "    \"vaxis\" \"[%.4f %.4f %.4f %.4f] %.4f\"\n", vDir[0], vDir[1], vDir[2], yoff, vertScale);

		fprintf(file, "    \"material\" \"%s\"\n", side->material);
		fprintf(file, "    \"rotation\" \"0\"\n");
		fprintf(file, "    \"lightmapscale\" \"%d\"\n", side->lightmapscale);
		fprintf(file, "    \"smoothing_groups\" \"0\"\n");
		fprintf(file, "  }\n");
	}
	fprintf(file, "}\n");
}

void exportEndBrushes(FILE* file)
{
	int len = dynList_size(brushes);
	for (int i = 0; i < len; i++)
	{
		Brush* brush = &brushes[i];
		if (brush->ent)
			continue;
		exportBrush(file, brush);
	}
}
