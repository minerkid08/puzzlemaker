#pragma once

#include "cglm/cglm.h"

typedef struct
{
  const char* material;
  int id;
  vec3 verts[4];
  vec3 normal;
  int lightmapscale;
  int texWidth;
  int texHeight;
	float rotation;
	float uoff;
	float uScale;
	float voff;
	float vScale;
  char alignment;
} Side;

typedef struct
{
  int id;
	vec3 pos;
	vec3 rot;
	vec3 bound1;
	vec3 bound2;
  char ent;
	char script;
  Side sides[6];
} Brush;

void exportStartBrushes();
void exportEndBrushes(FILE* file);
Brush* exportCreateBrush(vec3 start, vec3 end);
void exportBrush(FILE* file, Brush* brush);
Brush* getBrushArray();
