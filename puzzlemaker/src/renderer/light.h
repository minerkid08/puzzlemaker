#pragma once

typedef struct
{
	float position[3];
	float constant;
	float linear;
	float quadratic;
	float	cutoff;
	float _;
	float ambient[3];
	float dirX;
	float diffuse[3];
	float dirY;
	float specular[3];
	float dirZ;
} __attribute__((packed)) Light;

void initLights();
Light* addLight();
void updateShadows(Light* light);
void removeLight(Light* light);
void updateLightBuffer();
