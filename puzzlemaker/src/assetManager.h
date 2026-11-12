#pragma once

#include "renderer/mesh.h"

void assetManagerCleanup();
unsigned int assetManagerLoadTexture(const char* filename, char* flags);
Mesh* assetManagerLoadMesh(const char* filename);
