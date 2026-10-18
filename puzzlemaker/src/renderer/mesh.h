#pragma once

#include "cglm/types.h"
#include "packages/package.h"

typedef struct
{
  unsigned int vertexArray;
  unsigned int vertBuffer;
  unsigned int indexBuffer;
  unsigned int shader;
  unsigned int vertCount;
} Mesh;

void loadMesh(Package* package, const char* filename, Mesh* mesh);
