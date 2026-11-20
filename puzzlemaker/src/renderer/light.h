#pragma once

typedef struct
{
	float position[3];
	float constant;
	float linear;
	float quadratic;
	float	cutoff;
	int shadowMapIndex;
	float ambient[3];
	float dirX;
	float diffuse[3];
	float dirY;
	float specular[3];
	float dirZ;
	float transform[4][4];
} Light;

void initLights();
Light* addLight();
void updateShadows(Light* light);
void removeLight(Light* light);
void updateLightBuffer();
