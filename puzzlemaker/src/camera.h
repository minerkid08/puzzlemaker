#pragma once

#include "cglm/types.h"

extern float fovx;
extern float fovy;

extern mat4 projMat;

extern vec4 cameraPos;
extern vec4 cameraRot;

extern vec4 forward;
extern vec4 up;
extern vec4 right;

void initCamera();
void updateCamera();
