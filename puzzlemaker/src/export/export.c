#include "export/export.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "export/script/exportScript.h"
#include "item/item.h"
#include "ui/fileBrowser.h"
#include <stdio.h>

static char filename[256];

int exportMap()
{
	char* name = fileBrowserGetPath();
	exportStartEntities();
	exportStartBrushes();

	Item* items = getItemList();
	int len = dynList_size(items);
	if (len == 0)
		return 1;
	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		item->ioEnt = 0;
		if (item->def == 0)
			continue;
		if (item->def->exportScript)
		{
			if (runExportScript(item))
				return 1;
		}
		else
			item->def->callbacks->exportItem(item);
	}

	exportEntitiesProcessOutputs();

	exportMapSettings();
	exportVoxels();

	snprintf(filename, 256, "%s.vmf", name);
	printf("exporting '%s'\n", filename);
	FILE* file = fopen(filename, "wb");
	fprintf(file, R"(versioninfo
{
  "editorversion" "400"
  "editorbuild" "3325"
  "mapversion" "0"
  "formatversion" "100"
  "prefab" "0"
}
visgroups
{
}
viewsettings
{
  "bSnapToGrid" "1"
  "bShowGrid" "1"
  "bShowLogicalGrid" "0"
  "nGridSpacing" "64"
  "bShow3DGrid" "0"
}
)");

	fprintf(file, R"(world
{
	"id" "1"
	"mapversion" "1"
	"classname" "worldspawn"
	"skyname" "sky_black_nofog"
	"maxpropscreenwidth" "-1"
	"detailvbsp" "detail.vbsp"
	"detailmaterial" "detail/detailsprites"
	"maxblobcount" "250"
	"maxprojectedtextures" "8")");

	exportEndBrushes(file);
	fprintf(file, "\n}\n");
	exportEndEntities(file);
	fprintf(file, R"(
cameras
{
  "activecamera" "-1"
}
cordons
{
  "active" "0"
})");
	fclose(file);

	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		if (item->ioEnt)
			free((char*)item->ioEnt);
	}
	return 0;
}
