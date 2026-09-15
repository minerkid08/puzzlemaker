#pragma once

typedef struct 
{
  char* name;

  char regen;
  char boots;
  int portalGun;
  int paintGun;
  int maxHealth;
  int gameType;
} MapSettings;

extern MapSettings mapSettings;
