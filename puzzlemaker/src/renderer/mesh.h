#pragma once

#include "cglm/types.h"

typedef struct
{
  unsigned int vertexArray;
  unsigned int vertBuffer;
  unsigned int indexBuffer;
  unsigned int shader;
  unsigned int vertCount;
} Mesh;

void loadMesh(const char* filename, Mesh* mesh);
