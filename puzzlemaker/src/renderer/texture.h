#pragma once

#include "packages/package.h"

#define TEX_ALIGNMENT_NONE 0
#define TEX_ALIGNMENT_BOTTOM_LEFT 1
#define TEX_ALIGNMENT_BOTTOM_RIGHT 2
#define TEX_ALIGNMENT_TOP_LEFT 3
#define TEX_ALIGNMENT_TOP_RIGHT 4
#define TEX_ALIGNMENT_FIT 5

typedef struct 
{
  unsigned int tex;
  const char* filename;
  float size;
  char alignment;
} Texture;

unsigned int loadTexture(Package* package, const char* filename, char* flags);
