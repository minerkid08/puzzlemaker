#include "utils.h"
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
