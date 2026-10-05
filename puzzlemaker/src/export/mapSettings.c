#include "mapsettings.h"
#include "export/entity.h"
#include "voxel/voxel.h"
#include <stdio.h>
#include <string.h>

void getExportPos(vec3 pos)
{
	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				if (getVoxel(x, y, z)->solid == 0)
				{
					pos[0] = x + 0.5;
					pos[1] = y + 0.5;
					pos[2] = z + 0.5;
					return;
				}
			}
		}
	}
}

void exportMapSettings()
{
	vec3 pos;
	getExportPos(pos);
	Entity* ent = exportCreateEntity();

	memcpy(ent->pos, pos, sizeof(vec3));

	ent->rotation[0] = 0;
	ent->rotation[1] = 0;
	ent->rotation[2] = 0;
	ent->className = "info_portal_gamerules";
	ent->name = strdup("settings");

	char buf[32];
	snprintf(buf, 32, "%d", mapSettings.regen);
	exportEntityAddKvss(ent, "enableregen", buf);
	snprintf(buf, 32, "%d", mapSettings.boots);
	exportEntityAddKvss(ent, "equipboots", buf);
	snprintf(buf, 32, "%d", mapSettings.maxHealth);
	exportEntityAddKvss(ent, "maxhealth", buf);
	snprintf(buf, 32, "%d", mapSettings.portalGun);
	exportEntityAddKvss(ent, "equipportalgun", buf);
	snprintf(buf, 32, "%d", mapSettings.paintGun);
	exportEntityAddKvss(ent, "equippaintgun", buf);
	snprintf(buf, 32, "%d", mapSettings.gameType);
	exportEntityAddKvss(ent, "gametype", buf);

	PostProcessData* data = &mapSettings.postProcess;

	Entity* fogController = exportCreateEntity();
	fogController->name = strdup("fogController");
	fogController->className = "env_fog_controller";
	memcpy(fogController->pos, pos, sizeof(vec3));

	fogController->rotation[0] = 0;
	fogController->rotation[1] = 0;
	fogController->rotation[2] = 0;
	snprintf(buf, 32, "%.2f, %.2f, %.2f", data->primaryFogColor[0] * 255.0f, data->primaryFogColor[1] * 255.0f,
			 data->primaryFogColor[2] * 255.0f);
	exportEntityAddKvss(fogController, "fogcolor", buf);
	snprintf(buf, 32, "%.2f, %.2f, %.2f", data->secondaryFogColor[0] * 255.0f, data->secondaryFogColor[1] * 255.0f,
			 data->secondaryFogColor[2] * 255.0f);
	exportEntityAddKvss(fogController, "fogcolor2", buf);
	snprintf(buf, 32, "%.2f", data->fogStart);
	exportEntityAddKvss(fogController, "fogstart", buf);
	snprintf(buf, 32, "%.2f", data->fogEnd);
	exportEntityAddKvss(fogController, "fogend", buf);
	snprintf(buf, 32, "%.2f", data->fogDensity);
	exportEntityAddKvss(fogController, "fogmaxdensity", buf);

	Entity* tonemapper = exportCreateEntity();
	tonemapper->name = strdup("tonemapper");
	tonemapper->className = "env_tonemap_controller";
	memcpy(tonemapper->pos, pos, sizeof(vec3));

	tonemapper->rotation[0] = 0;
	tonemapper->rotation[1] = 0;
	tonemapper->rotation[2] = 0;

	Entity* logicAuto = exportCreateEntity();
	logicAuto->name = strdup("auto");
	logicAuto->className = "logic_auto";
	memcpy(logicAuto->pos, pos, sizeof(vec3));

	logicAuto->rotation[0] = 0;
	logicAuto->rotation[1] = 0;
	logicAuto->rotation[2] = 0;

	snprintf(buf, 32, "%.2f", data->minExposure);
	exportEntityAddRawOutput(logicAuto, "OnMapSpawn", "tonemapper", "SetAutoExposureMin", buf, 0);
	snprintf(buf, 32, "%.2f", data->maxExposure);
	exportEntityAddRawOutput(logicAuto, "OnMapSpawn", "tonemapper", "SetAutoExposureMax", buf, 0);
	snprintf(buf, 32, "%.2f", data->tonemapRate);
	exportEntityAddRawOutput(logicAuto, "OnMapSpawn", "tonemapper", "SetTonemapRate", buf, 0);
	snprintf(buf, 32, "%.2f", data->brightPixelPercent);
	exportEntityAddRawOutput(logicAuto, "OnMapSpawn", "tonemapper", "SetTonemapPercentBrightPixels", buf, 0);
}
