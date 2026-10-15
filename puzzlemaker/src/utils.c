#include "utils.h"
#include "cglm/types.h"
#include "ui/itemPanel.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

ivec3 dirs[] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};

extern char** stackTrace;
extern char* err;
extern char errorState;

void errorf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char buf[256];
	vsnprintf(buf, 256, fmt, va);
	err = strdup(buf);
	errorState = 1;
	startErrorLoop();
}

char startsWith(const char* a, const char* b)
{
	int l = strlen(b);
	for (int i = 0; i < l; i++)
	{
		if (a[i] == 0)
			return 0;
		if (a[i] != b[i])
			return 0;
	}
	return 1;
}

int strncatf(char* buf, int size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int len = strlen(buf);
	if (len >= size)
		return 0;
	return vsnprintf(buf + len, size - len, fmt, va);
}

char* strdupf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int len = vsnprintf(0, 0, fmt, va);
	char* buf = malloc(len + 1);
	va_start(va, fmt);
	vsnprintf(buf, len + 1, fmt, va);
	return buf;
}

void getEulerAngles(mat4 matrix, vec3 out)
{
	mat4 mat;
	memcpy(mat, matrix, sizeof(mat4));
	mat4 m2;
	glm_mat4_identity(m2);
	glm_rotate_y(m2, glm_rad(90), m2);
	glm_mat4_mul(mat, m2, mat);

	vec3 forward = {-mat[0][2], -mat[0][0], -mat[0][1]};
	vec3 up = {mat[1][2], mat[1][0], mat[1][1]};
	vec3 left = {mat[2][2], mat[2][0], mat[2][1]};

	float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

	// enough here to get angles?
	if (xyDist > 0.001f)
	{
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		out[1] = (atan2f(forward[1], forward[0]));

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		out[0] = (atan2f(-forward[2], xyDist));

		// (roll)	z = ATAN( left.z, up.z );
		out[2] = (atan2f(left[2], up[2]));
	}
	else // forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		out[1] = (atan2f(-left[0], left[1]));

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		out[0] = (atan2f(-forward[2], xyDist));

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		out[2] = 0;
	}
}
