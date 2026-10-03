#pragma once

#include "cglm/types.h"

typedef struct 
{
	const char* name;
	float maxExposure;
	float minExposure;
	float tonemapRate;
	float brightPixelPercent;
	vec3 primaryFogColor;
	vec3 secondaryFogColor;
	float fogStart;
	float fogEnd;
	float fogDensity;
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
