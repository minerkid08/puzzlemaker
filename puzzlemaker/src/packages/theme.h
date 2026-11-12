#pragma once

#include "antline/antline.h"
#include "mapsettings.h"
#include "voxel/voxelConfig.h"

typedef struct
{
  const char* name;
  VoxelConfig voxelCfg;
  AntlineConfig antlineCfg;
  PostProcessData* postProcessPresets;
} Theme;

extern Theme* currentTheme;
extern Theme* themeList;

void loadTheme(const char* filename);
void setTheme(Theme* theme);
