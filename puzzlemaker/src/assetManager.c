#include "assetManager.h"
#include "dynList.h"
#include "glad/glad.h"
#include "packages/package.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	const char* name;
	unsigned int id;
	char flags;
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

void assetManagerCleanup()
{
	int len = dynList_size(textures);
	for (int i = 0; i < len; i++)
	{
		TextureAsset* asset = &textures[i];
		free((char*)asset->name);
		glDeleteTextures(1, &textures[i].id);
	}
	dynList_free(textures);

	len = dynList_size(meshes);
	for (int i = 0; i < len; i++)
	{
		MeshAsset* meshAsset = &meshes[i];
		free((char*)meshAsset->name);
		Mesh* mesh = meshAsset->mesh;
		glDeleteBuffers(1, &mesh->vertBuffer);
		glDeleteBuffers(1, &mesh->indexBuffer);
	}
	dynList_free(meshes);
}

unsigned int assetManagerLoadTexture(const char* filename, char* flags)
{
	int offset = 0;

	Package* package = processPackageString(currentPackage, filename, &offset);
	const char* file = filename + offset;

	int len = dynList_size(textures);
	char* textureName = strdupf("packages/%s/assets/materials/%s", package->name, file);
	for (int i = 0; i < len; i++)
	{
		TextureAsset* asset = &textures[i];
		if (strcmp(asset->name, textureName) == 0)
		{
			free(textureName);
			if (flags)
				*flags |= asset->flags;
			return asset->id;
		}
	}

	dynList_resize((void**)&textures, len + 1);

	TextureAsset* asset = &textures[len];
	asset->name = textureName;

	printf("[asset manager] loading texture '%s'\n", file);

	asset->flags = 0;
	asset->id = loadTexture(package, file, &asset->flags);
	if (flags)
		*flags |= asset->flags;
	return asset->id;
}

Mesh* assetManagerLoadMesh(const char* filename)
{
	int offset = 0;

	Package* package = processPackageString(currentPackage, filename, &offset);
	const char* file = filename + offset;

	char* meshName = strdupf("packages/%s/assets/models/%s", package->name, file);
	int len = dynList_size(meshes);
	for (int i = 0; i < len; i++)
	{
		MeshAsset* asset = &meshes[i];
		if (strcmp(asset->name, meshName) == 0)
			return asset->mesh;
	}

	dynList_resize((void**)&meshes, len + 1);

	MeshAsset* asset = &meshes[len];
	asset->name = meshName;

	printf("[asset manager] loading mesh '%s'\n", file);

	asset->mesh = malloc(sizeof(Mesh));

	loadMesh(package, file, asset->mesh);
	return asset->mesh;
}
