#pragma once

#include "cglm/types.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"

void initRenderer();
void rendererCleanup();

void setBackfaceCull(char enable);
void setDepthTest(char enable);
void setRenderPass(int i);

void setTransform(mat4 transform);
void setProjMat(mat4 mat);
void setCamMat(mat4 mat);

void genVerts(int direction, vec3 start, vec2 size, vec3* positions);

void drawRect(vec3* verts, Texture* texture);
void drawRectTint(vec3* verts, vec4 tint, Texture* texture);
void drawRectTile(vec3* verts, float uStart, float uEnd, float vStart, float vEnd, Texture* texture);

void drawRectTintInv(vec3* positions, vec4 tint, Texture* texture);
void drawRectTileInv(vec3* verts, float uStart, float uEnd, float vStart, float vEnd, Texture* texture);

void endFrame();

mat4* getProjMat();
mat4* getCamMat();
