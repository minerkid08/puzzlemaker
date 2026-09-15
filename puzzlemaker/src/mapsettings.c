#include "mapsettings.h"
#include <stdlib.h>

MapSettings mapSettings;

static __attribute__((constructor)) void init()
{
  mapSettings.name = malloc(256);
  mapSettings.boots = 0;
  mapSettings.regen = 1;
  mapSettings.portalGun = 3;
  mapSettings.paintGun = 0;
  mapSettings.maxHealth = 100;
  mapSettings.gameType = 0;
}
