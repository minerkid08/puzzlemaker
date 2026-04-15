#pragma once

#include "cglm/types.h"

#define min(x, y) ((x) > (y) ? (y) : (x))
#define max(x, y) ((x) < (y) ? (y) : (x))
#define absf(x) ((x) < 0.0f ? -(x) : (x))

#define DIR_POS_X 0
#define DIR_NEG_X 1
#define DIR_POS_Y 2
#define DIR_NEG_Y 3
#define DIR_POS_Z 4
#define DIR_NEG_Z 5

extern ivec3 dirs[6];

char* copyString(const char* str);
