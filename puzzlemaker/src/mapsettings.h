#pragma once

#include "cglm/types.h"

typedef struct 
{
	const char* name;
	char enableTonemap;
	float maxExposure;
	float minExposure;
	float tonemapRate;
	float brightPixelPercent;

	char enableFog;
	vec3 primaryFogColor;
	vec3 secondaryFogColor;
	float fogStart;
	float fogEnd;
	float fogDensity;
	
	char enableColorCorrection;
	float minFalloff;
	float maxFalloff;
	float maxWeight;
	float fadeInDuration;
	float fadeOutDuration;
	char* filename;
} PostProcessData;

typedef struct 
{
  char* name;

  char regen;
  char boots;
  int portalGun;
  int paintGun;
  int maxHealth;
  int gameType;

	int postProcessPreset;
	PostProcessData postProcess;

} MapSettings;

extern MapSettings mapSettings;

void loadMapSettingsPresets();
