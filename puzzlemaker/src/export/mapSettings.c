#include "mapsettings.h"
#include "export/entity.h"
#include "item/item.h"
#include <stdio.h>
#include <string.h>

void exportMapSettings()
{
  Entity* ent = exportCreateEntity();

  memcpy(ent->pos, getItemList()[0].pos, sizeof(vec3));

  ent->rotation[0] = 0;
  ent->rotation[1] = 0;
  ent->rotation[2] = 0;
  ent->className = "info_portal_gamerules";
  ent->name = strdup("settings");

  char buf[8];
  snprintf(buf, 8, "%d", mapSettings.regen);
  exportEntityAddKvss(ent, "enableregen", buf);
  snprintf(buf, 8, "%d", mapSettings.boots);
  exportEntityAddKvss(ent, "equibboots", buf);
  snprintf(buf, 8, "%d", mapSettings.maxHealth);
  exportEntityAddKvss(ent, "maxhealth", buf);
  snprintf(buf, 8, "%d", mapSettings.portalGun);
  exportEntityAddKvss(ent, "equipportalgun", buf);
  snprintf(buf, 8, "%d", mapSettings.paintGun);
  exportEntityAddKvss(ent, "equippaintgun", buf);
  snprintf(buf, 8, "%d", mapSettings.gameType);
  exportEntityAddKvss(ent, "gametype", buf);
}
