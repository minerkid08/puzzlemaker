#include "export/export.h"
#include "antline/antline.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "export/overlay.h"
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
	exportStartOverlays();

	Item* items = getItemList();
	int len = dynList_size(items);
	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		if (!isItemValid(item))
			continue;
		for (int j = 0; j < dynList_size(item->def->kvs); j++)
		{
			ItemKv* kv = &item->kv[j];
			int type = kv->def->type & ~(TYPE_INSTANCE);
			if (type != TYPE_PICKER)
				continue;
			if (kv->value.i == -1 && kv->def->dropValues->b)
			{
				writeMessagef("Error: required item kv %s is not set\n", kv->def->displayName);
				writeMessagef("in item %s %d\n", item->def->name, item->index);
				return 1;
			}
		}
	}

	char requiresPaint = 0;
	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		if (!isItemValid(item))
			continue;
		item->ioEnt = 0;
		if (item->def->requiresPaint)
			requiresPaint = 1;
		if (item->def->exportScript)
		{
			if (runExportScript(item))
				return 1;
		}
		else
			item->def->callbacks->exportItem(item);
	}

	antlineExport();
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

	fprintf(file, "  \"paintinmap\" \"%d\"\n", requiresPaint);

	exportEndBrushes(file);
	fprintf(file, "\n}\n");
	exportEndEntities(file);
	exportEndOverlays(file);
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
		if (!isItemValid(item))
			continue;
		if (item->ioEnt)
			free((char*)item->ioEnt);
		item->ioEnt = 0;
	}
	return 0;
}
