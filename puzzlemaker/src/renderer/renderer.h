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

void panelDrawRect(vec2 start, vec2 end, unsigned int texture);
void drawRect(vec3 v1, vec3 v2, vec3 v3, vec3 v4, unsigned int texture);
void panelEndFrame(mat4 transform, char backfaceCull);

mat4* getProjMat();
mat4* getCamMat();
