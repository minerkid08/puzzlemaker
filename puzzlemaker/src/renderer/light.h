#pragma once

typedef struct
{
	float transform[4][4];
	float position[3];
	float constant;
	float ambient[3];
	float linear;
	float diffuse[3];
	float quadratic;
	float specular[3];
	float	cutoff;
	float dirX;
	float dirY;
	float dirZ;
	int shadowMapIndex;
} Light;

void initLights();
Light* addLight();
void updateShadows(Light* light);
void removeLight(Light* light);
void updateLightBuffer();
