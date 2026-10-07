#pragma once

#include "cglm/types.h"
#include "renderer/mesh.h"

void initRenderer();
void rendererCleanup();
void endFrame();
void drawVerts(vec3* verts, vec4 tint, char portalable);
void drawMesh(Mesh* mesh, unsigned int texture, mat4 transform);
void setProjMat(mat4 mat);
void setCamMat(mat4 mat);
void bindTexture(unsigned int texture);
void bindVoxelTextures(unsigned int black, unsigned int white, unsigned int miniBlack, unsigned int miniWhite);

void panelDrawRect(vec2 start, vec2 end, unsigned int texture);
void overlayDrawRect(vec2 start, vec2 end, unsigned int texture, int tile);
void drawRect(vec3 v1, vec3 v2, vec3 v3, vec3 v4, unsigned int texture);
void panelEndFrame(mat4 transform, char backfaceCull, char depthTest);

mat4* getProjMat();
mat4* getCamMat();
