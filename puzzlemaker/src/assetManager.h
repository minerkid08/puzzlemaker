#pragma once

#include "renderer/mesh.h"

void assetManagerCleanup();
unsigned int assetManagerLoadTexture(const char* filename);
Mesh* assetManagerLoadMesh(const char* filename);
