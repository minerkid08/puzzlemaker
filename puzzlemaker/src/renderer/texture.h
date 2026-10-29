#pragma once

#include "packages/package.h"
#define TEX_ALIGNMENT_NONE 0
#define TEX_ALIGNMENT_FIT 1

typedef struct 
{
  unsigned int tex;
  const char* filename;
  float size;
  char alignment;
} Texture;

unsigned int loadTexture(Package* package, const char* filename, char* flags);
