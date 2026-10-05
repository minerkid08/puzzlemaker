#pragma once

#include "cglm/types.h"
#include <stdio.h>
typedef struct
{
	const char* name;
	vec3 pos;
	vec3 rotation;
	vec2 size;
	vec2 tile;
	const char* texture;
	vec4 tint;
} Overlay;

Overlay* getOverlayList();
void exportStartOverlays();
Overlay* exportCreateOverlay();
void exportEndOverlays(FILE* file);
