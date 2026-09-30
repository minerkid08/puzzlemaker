#include "utils.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

ivec3 dirs[] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};

char* copyString(const char* str)
{
	int len = strlen(str);

	char* buf = malloc(len + 1);
	strncpy(buf, str, len + 1);
	return buf;
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
