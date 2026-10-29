#pragma once

#include "cglm/types.h"

#define CORNER_MODE_AUTO 1
#define CORNER_MODE_ENABLED 2
#define CORNER_MODE_DISABLED 3

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
	mat4 transform;
	vec4 quat;
	vec3 pos;
	vec3 rot;
	int len;
	int cornerMode;
	char snapDir;
} AntlineSegment;

typedef struct
{
	AntlineSegment baseSegment;
	AntlineSegment* segments;
	int id;
	int prevId;
	char hovered;
	char hasCheck;
} Antline;

extern AntlineConfig* antlineConfig;

AntlineConfig* getAntlineConfig();
Antline* addAntline();
void removeAntline(Antline* antline);
char isAntlineValid(Antline* antline);
void renderAntlines();
void renderAntlineHover();
void antlineUpdateTransformRot(AntlineSegment* antline);
void antlineUpdateTransform(AntlineSegment* antline);
Antline* getIntersectingAntline(vec3 pos, AntlineSegment** seg);
void antlineExport();
