#pragma once

#include "cglm/types.h"

unsigned int makeShader();
void setUniformMat4(unsigned int id, const char* name, mat4 value);
void setUndformi(unsigned int id, const char* name, int value);
