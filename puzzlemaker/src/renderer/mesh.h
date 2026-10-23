#pragma once

#include "cglm/types.h"
#include "packages/package.h"

typedef struct
{
	int texture;
	int alphaTex;
	vec4 tint;
} MeshMaterial;

typedef struct
{
	unsigned int* textures;
	int textureCount;
	MeshMaterial* materials;
	int materialCount;
  unsigned int vertexArray;
  unsigned int vertBuffer;
  unsigned int indexBuffer;
  unsigned int vertCount;
	char flags;
} Mesh;

void initMeshRenderer();
void loadMesh(Package* package, const char* filename, Mesh* mesh);
void renderMesh(Mesh* mesh, mat4 transform);
