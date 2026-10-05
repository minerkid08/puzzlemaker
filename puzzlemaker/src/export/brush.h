#pragma once

#include "cglm/cglm.h"

typedef struct
{
  int id;
  vec3 verts[4];
  vec2 uvs[4];
  const char* material;
  int lightmapscale;
  int texWidth;
  int texHeight;
  char fit;
	vec3 normal;
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
