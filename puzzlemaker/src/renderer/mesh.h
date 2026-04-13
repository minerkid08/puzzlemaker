#pragma once

typedef struct
{
  unsigned int vertexArray;
  unsigned int vertBuffer;
  unsigned int indexBuffer;
  unsigned int shader;
  unsigned int vertCount;
} Mesh;

Mesh loadMesh(const char* filename);
