#include "assetManager.h"
#include "dynList.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	const char* name;
	unsigned int id;
} TextureAsset;

typedef struct
{
	const char* name;
	Mesh* mesh;
} MeshAsset;

TextureAsset* textures;
MeshAsset* meshes;

__attribute__((constructor)) static void init()
{
  textures = dynList_new(0, sizeof(TextureAsset));
  dynList_reserve((void**)&textures, 5);

  meshes = dynList_new(0, sizeof(MeshAsset));
  dynList_reserve((void**)&meshes, 5);
}

unsigned int assetManagerLoadTexture(const char* filename)
{
  int len = dynList_size(textures);
	for (int i = 0; i < len; i++)
  {
    TextureAsset* asset = &textures[i];
    if(strcmp(asset->name, filename) == 0)
      return asset->id;
  }

  dynList_resize((void**)&textures, len + 1);

  TextureAsset* asset = &textures[len];
  asset->name = strdup(filename);

  printf("[asset manager] loading texture '%s'\n", filename);

  asset->id = loadTexture(filename);
  return asset->id;
}

Mesh* assetManagerLoadMesh(const char* filename)
{
  int len = dynList_size(meshes);
	for (int i = 0; i < len; i++)
  {
    MeshAsset* asset = &meshes[i];
    if(strcmp(asset->name, filename) == 0)
      return asset->mesh;
  }

  dynList_resize((void**)&meshes, len + 1);

  MeshAsset* asset = &meshes[len];
  asset->name = strdup(filename);

  printf("[asset manager] loading mesh '%s'\n", filename);

  asset->mesh = malloc(sizeof(Mesh));

  *asset->mesh = loadMesh(filename);
  return asset->mesh;
}
