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
} Side;

typedef struct
{
  int id;
  char ent;
  Side sides[6];
} Brush;

void exportStartBrushes();
void exportEndBrushes(FILE* file);
Brush* exportCreateBrush(vec3 start, vec3 end);
void exportBrush(FILE* file, Brush* brush);
