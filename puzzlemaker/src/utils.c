#include "utils.h"
#include "ui/itemPanel.h"
#include <math.h>
#include <stdarg.h>
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

void getEulerAngles(mat4 mat, vec3 out)
{
	double t1 = atan2(-mat[0][2], mat[0][0]);
	double c2 = sqrt(mat[1][1] * mat[1][1] + mat[2][1] * mat[2][1]);
	double t2 = atan2(mat[0][1], c2);
	double s1 = sin(t1);
	double c1 = cos(t1);
	double t3 = atan2(s1 * mat[1][0] + c1 * mat[1][2], s1 * mat[2][0] + c1 * mat[2][2]);
	out[1] = t1;
	out[2] = t2;
	out[0] = t3;
}
