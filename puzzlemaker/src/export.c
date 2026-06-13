#include "export/export.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "item/entityItem.h"
#include "item/item.h"
#include "item/panel.h"
#include <stdio.h>

static char filename[64];

void export2(const char* name)
{
	exportStartEntities();
	exportStartBrushes();

	Item* items = getItemList();
	int len = dynList_size(items);
	for (int i = 0; i < len; i++)
	{
		Item* item = &items[i];
		if (item->def->type == ITEM_TYPE_ENTITY)
			entityItemExport(item);
		if (item->def->type == ITEM_TYPE_PANEL)
			panelItemExport(item);
	}

	exportVoxels();

	snprintf(filename, 64, "maps/%s.vmf", name);
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
	fprintf(file, "\n}");
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
}
