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
		if (item->def->flags & ITEM_FLAGS_REQUIRES_PAINT)
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
	fprintf(file, "versioninfo\n");
	fprintf(file, "{\n");
	fprintf(file, "  \"editorversion\" \"400\"\n");
	fprintf(file, "  \"editorbuild\" \"3325\"\n");
	fprintf(file, "  \"mapversion\" \"0\"\n");
	fprintf(file, "  \"formatversion\" \"100\"\n");
	fprintf(file, "  \"prefab\" \"0\"\n");
	fprintf(file, "}\n");
	fprintf(file, "visgroups\n");
	fprintf(file, "{\n");
	fprintf(file, "}\n");
	fprintf(file, "viewsettings\n");
	fprintf(file, "{\n");
	fprintf(file, "  \"bSnapToGrid\" \"1\"\n");
	fprintf(file, "  \"bShowGrid\" \"1\"\n");
	fprintf(file, "  \"bShowLogicalGrid\" \"0\"\n");
	fprintf(file, "  \"nGridSpacing\" \"64\"\n");
	fprintf(file, "  \"bShow3DGrid\" \"0\"\n");
	fprintf(file, "}\n");

	fprintf(file, "world\n");
	fprintf(file, "{\n");
	fprintf(file, "	\"id\" \"1\"\n");
	fprintf(file, "	\"mapversion\" \"1\"\n");
	fprintf(file, "	\"classname\" \"worldspawn\"\n");
	fprintf(file, "	\"skyname\" \"sky_black_nofog\"\n");
	fprintf(file, "	\"maxpropscreenwidth\" \"-1\"\n");
	fprintf(file, "	\"detailvbsp\" \"detail.vbsp\"\n");
	fprintf(file, "	\"detailmaterial\" \"detail/detailsprites\"\n");
	fprintf(file, "	\"maxblobcount\" \"250\"\n");
	fprintf(file, "	\"maxprojectedtextures\" \"8\"\n");

	fprintf(file, "  \"paintinmap\" \"%d\"\n", requiresPaint);

	exportEndBrushes(file);
	fprintf(file, "\n}\n");
	exportEndEntities(file);
	exportEndOverlays(file);
	fprintf(file, "\n");
	fprintf(file, "cameras\n");
	fprintf(file, "{\n");
	fprintf(file, "  \"activecamera\" \"-1\"\n");
	fprintf(file, "}\n");
	fprintf(file, "cordons\n");
	fprintf(file, "{\n");
	fprintf(file, "  \"active\" \"0\"\n");
	fprintf(file, "}");
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
