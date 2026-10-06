#pragma once

#include "cglm/types.h"

typedef struct
{
	int editorAntlineTex;
	int editorAntlineActiveTex;
	int editorCheckTex;
	int editorCheckActiveTex;

	int dotSize;
	int checkSize;
	float editorDotSize;
	float editorCheckSize;

	const char* antlineTex;
	int antlineWidth;
	int antlineHeight;

	const char* antlineCornerTex;
	int antlineCornerWidth;
	int antlineCornerHeight;

	const char* antlineCheckTex;
} AntlineConfig;

typedef struct
{
	vec3 pos;
	vec3 rot;
	int len;
	vec4 quat;
	mat4 transform;
	mat4 invTransform;
	char snapDir;
} AntlineSegment;

typedef struct
{
	int id;
	int prevId;
	char hovered;
	AntlineSegment* segments;
	char hasCheck;
	AntlineSegment baseSegment;
} Antline;

void loadAntlineConfig();
Antline* addAntline();
void removeAntline(Antline* antline);
char isAntlineValid(Antline* antline);
void renderAntlines();
void antlineUpdateTransformRot(AntlineSegment* antline);
void antlineUpdateTransform(AntlineSegment* antline);
Antline* getIntersectingAntline(vec3 pos, AntlineSegment** seg);
void antlineExport();
