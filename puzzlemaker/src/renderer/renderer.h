#pragma once

#include "cglm/types.h"

void initRenderer();
void endFrame();
void drawVerts(vec3* verts, vec4 tint);
void setProjMat(mat4 mat);
void setCamMat(mat4 mat);
