#pragma once

#include "cglm/types.h"

static inline void transformPos(vec3 pos)
{
	float tmp = pos[0];
	pos[0] = pos[2] * 64 - 8192;
	pos[2] = pos[1] * 64 - 8192;
	pos[1] = tmp * 64 - 8192;
}

static inline void transformPosTo(vec3 pos, vec3 out)
{
	out[0] = pos[2] * 64 - 8192;
	out[1] = pos[0] * 64 - 8192;
	out[2] = pos[1] * 64 - 8192;
}

static inline void transformVec(vec3 pos)
{
	float tmp = pos[0];
	pos[0] = pos[2];
	pos[2] = pos[1];
	pos[1] = tmp;
}

static inline void transformVecTo(vec3 pos, vec3 out)
{
	out[0] = pos[2];
	out[2] = pos[1];
	out[1] = pos[0];
}
