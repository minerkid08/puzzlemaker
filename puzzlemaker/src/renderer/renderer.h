#pragma once

#include "cglm/types.h"
#include "renderer/mesh.h"

void initRenderer();
void endFrame();
void drawVerts(vec3* verts, vec4 tint);
void drawMesh(Mesh* mesh, unsigned int texture, mat4 transform);
void setProjMat(mat4 mat);
void setCamMat(mat4 mat);
void bindTexture(unsigned int texture);

mat4* getProjMat();
mat4* getCamMat();
