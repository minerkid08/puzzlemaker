#include "cglm/types.h"

void initDebug();
void debugCleanup();
void drawDebugRect(vec3 a, vec3 b, mat4 transform, char color);
void drawDebugRectAntline(vec3 a, vec3 b, mat4 transform);
void drawDebugPoint(vec3 a, vec3 b, char col);
