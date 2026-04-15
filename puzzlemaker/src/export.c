#include "cglm/types.h"
#include "dynList.h"
#include "item.h"
#include "utils.h"
#include "voxel.h"
#include <stdio.h>

int id = 1;

static const char* uaxis[] = {"[0 1 0 0] 0.25",	 "[0 -1 0 0] 0.25", "[1 0 0 0] 0.25",
							  "[-1 0 0 0] 0.25", "[-1 0 0 0] 0.25", "[1 0 0 0] 0.25"};

static const char* vaxis[] = {"[0 0 -1 0] 0.25", "[0 0 -1 0] 0.25", "[0 -1 0 0] 0.25",
							  "[0 -1 0 0] 0.25", "[0 0 -1 0] 0.25", "[0 0 -1 0] 0.25"};

#define MAT_NODRAW "TOOLS/TOOLSNODRAW"

#define MAT_FLOOR "METAL/BLACK_FLOOR_METAL_001C"
#define MAT_WALL "METAL/BLACK_WALL_METAL_002A"
#define MAT_CEILING "METAL/BLACK_CEILING_METAL_001A"

#define SIDEINDENT "      "

static char filename[64];

void addSide(FILE* file, ivec3 verts[4], int i, int dir, const char* mat)
{
	fprintf(file, "\n    side\n    {\n");
	fprintf(file, SIDEINDENT "\"id\" \"%d\"\n", i + 1);

	char planeBuf[50];
	snprintf(planeBuf, 50, "(%d %d %d) (%d %d %d) (%d %d %d)", -verts[0][0] * 64, verts[0][2] * 64, verts[0][1] * 64,
			 -verts[2][0] * 64, verts[2][2] * 64, verts[2][1] * 64, -verts[3][0] * 64, verts[3][2] * 64,
			 verts[3][1] * 64);

	fprintf(file, SIDEINDENT "\"plane\" \"%s\"\n", planeBuf);

	fprintf(file, SIDEINDENT "point_data\n");
	fprintf(file, SIDEINDENT "{\n");
	fprintf(file, SIDEINDENT "  \"numpts\" \"4\"\n");

	fprintf(file, SIDEINDENT "  \"point\" \"0 %d %d %d\"\n", -verts[0][0] * 64, verts[0][2] * 64, verts[0][1] * 64);
	fprintf(file, SIDEINDENT "  \"point\" \"1 %d %d %d\"\n", -verts[2][0] * 64, verts[2][2] * 64, verts[2][1] * 64);
	fprintf(file, SIDEINDENT "  \"point\" \"2 %d %d %d\"\n", -verts[3][0] * 64, verts[3][2] * 64, verts[3][1] * 64);
	fprintf(file, SIDEINDENT "  \"point\" \"3 %d %d %d\"\n", -verts[1][0] * 64, verts[1][2] * 64, verts[1][1] * 64);

	fprintf(file, SIDEINDENT "}\n");
	fprintf(file, SIDEINDENT "\"material\" \"%s\"\n", mat);
	fprintf(file, SIDEINDENT "\"uaxis\" \"%s\"\n", uaxis[dir]);
	fprintf(file, SIDEINDENT "\"vaxis\" \"%s\"\n", vaxis[dir]);
	fprintf(file, SIDEINDENT "\"rotation\" \"0\"\n");
	fprintf(file, SIDEINDENT "\"lightmapscale\" \"16\"\n");
	fprintf(file, SIDEINDENT "\"smoothing_groups\" \"0\"\n");
	fprintf(file, "    }");
}

void generateSolid(FILE* file, Voxel* voxel, ivec3 pos)
{
	fprintf(file, R"(
  solid
  {
    "id" "%d")",
			id++);

	int x = pos[0];
	int y = pos[1];
	int z = pos[2];

	char* mat = MAT_NODRAW;
	if (z + 1 < MAP_SIZE)
	{
		Voxel* v2 = getVoxel(x, y, z + 1);
		if (!v2->solid)
			mat = MAT_WALL;
	}
	{
		ivec3 verts[4] = {{x, y, z + 1}, {x + 1, y, z + 1}, {x, y + 1, z + 1}, {x + 1, y + 1, z + 1}};
		addSide(file, verts, 1, DIR_POS_Z, mat);
	}

	mat = MAT_NODRAW;
	if (z - 1 >= 0)
	{
		Voxel* v2 = getVoxel(x, y, z - 1);
		if (!v2->solid)
			mat = MAT_WALL;
	}
	{
		ivec3 verts[4] = {{x, y, z}, {x, y + 1, z}, {x + 1, y, z}, {x + 1, y + 1, z}};
		addSide(file, verts, 2, DIR_NEG_Z, mat);
	}

	mat = MAT_NODRAW;
	if (x + 1 < MAP_SIZE)
	{
		Voxel* v2 = getVoxel(x + 1, y, z);
		if (!v2->solid)
			mat = MAT_WALL;
	}
	{
		ivec3 verts[4] = {{x + 1, y, z}, {x + 1, y + 1, z}, {x + 1, y, z + 1}, {x + 1, y + 1, z + 1}};
		addSide(file, verts, 3, DIR_POS_X, mat);
	}

	mat = MAT_NODRAW;
	if (x - 1 >= 0)
	{
		Voxel* v2 = getVoxel(x - 1, y, z);
		if (!v2->solid)
			mat = MAT_WALL;
	}
	{
		ivec3 verts[4] = {{x, y, z}, {x, y, z + 1}, {x, y + 1, z}, {x, y + 1, z + 1}};
		addSide(file, verts, 4, DIR_NEG_X, mat);
	}

	mat = MAT_NODRAW;
	if (y + 1 < MAP_SIZE)
	{
		Voxel* v2 = getVoxel(x, y + 1, z);
		if (!v2->solid)
			mat = MAT_FLOOR;
	}
	{
		ivec3 verts[4] = {{x, y + 1, z}, {x, y + 1, z + 1}, {x + 1, y + 1, z}, {x + 1, y + 1, z + 1}};
		addSide(file, verts, 5, DIR_POS_Y, mat);
	}

	mat = MAT_NODRAW;
	if (y - 1 >= 0)
	{
		Voxel* v2 = getVoxel(x, y - 1, z);
		if (!v2->solid)
			mat = MAT_CEILING;
	}
	{
		ivec3 verts[4] = {{x, y, z}, {x + 1, y, z}, {x, y, z + 1}, {x + 1, y, z + 1}};
		addSide(file, verts, 6, DIR_NEG_Y, mat);
	}

	fprintf(file, R"(
    editor
    {
      "color" "0 130 143"
      "visgroupsshown" "1"
      "visgroupsautoshown" "1"
    }
  })");
}

void generateItem(FILE* file, Item* item)
{
	char buf[100];
	snprintf(buf, 100, "%s%d", item->def->name, item->index);

	float x = item->pos[0] + item->def->offset[0];
	float y = item->pos[1] + item->def->offset[1];
	float z = item->pos[2] + item->def->offset[2];

	if (item->def->type == ENTITY_TYPE_INSTANCE)
	{
		fprintf(file, R"(
entity
{
  "id" "%d"
  "classname" "func_instance"
  "origin" "%f %f %f"
  "angles" "%f %f %f"
  "targetname" "%s"
  "file" "%s")",
				item->index, -x * 64, z * 64, y * 64, item->dir[0], item->dir[1], item->dir[2], buf,
				item->def->instanceName);
	}
	else
	{
		fprintf(file, R"(
entity
{
  "id" "%d"
  "classname" "%s"
  "origin" "%f %f %f"
  "angles" "%f %f %f"
  "targetname" "%s")",
				item->index, item->def->instanceName, -x * 64, z * 64, y * 64, item->dir[0], item->dir[1], item->dir[2],
				buf);
	}

	int l = dynList_size(item->def->staticKvs);
	for (int i = 0; i < l; i++)
		fprintf(file, "\n %s", item->def->staticKvs[i]);

	l = dynList_size(item->def->kvs);
	for (int i = 0; i < l; i++)
	{
		ItemKvDef* def = &item->def->kvs[i];
		ItemKv* kv = &item->kv[i];
		if (def->type == TYPE_INT)
			fprintf(file, "\n  \"%s\" \"%d\"", def->name, kv->value.i);
		if (def->type == TYPE_BOOL)
			fprintf(file, "\n  \"%s\" \"%d\"", def->name, (int)kv->value.b);
		if (def->type & TYPE_DROPDOWN)
		{
			int type = (def->type & (~TYPE_DROPDOWN));
			if (type == TYPE_STRING)
				fprintf(file, "\n  \"%s\" \"%s\"", def->name, kv->def->dropValues[kv->value.i].s);
			if (type == TYPE_INT)
				fprintf(file, "\n  \"%s\" \"%d\"", def->name, kv->def->dropValues[kv->value.i].i);
		}
	}

	int outputLen = dynList_size(item->outputs);
	if (outputLen > 0)
	{
		fprintf(file, "\n  connections\n  {\n");
		for (int i = 0; i < outputLen; i++)
		{
			ItemOutput* output = &item->outputs[i];
			snprintf(buf, 100, "%s%d", output->entity->def->name, output->entity->index);
			if (output->inverted)
			{
				fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
						output->input->falseInput);
				fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->falseOutput, buf,
						output->input->trueInput);
			}
			else
			{
				fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
						output->input->trueInput);
				fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->falseOutput, buf,
						output->input->falseInput);
			}
		}
		fprintf(file, "  }");
	}
	fprintf(file, "\n}");
}

void exportMap(const char* name)
{
	snprintf(filename, 64, "%s.vmf", name);
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

	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->solid)
				{
					char shouldExport = 0;
					if (z + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y, z + 1);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (z - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y, z - 1);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (x + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x + 1, y, z);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (x - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x - 1, y, z);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (y + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y + 1, z);
						if (!v2->solid)
							shouldExport = 1;
					}

					if (y - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y - 1, z);
						if (!v2->solid)
							shouldExport = 1;
					}
					if (shouldExport)
					{
						ivec3 pos = {x, y, z};
						generateSolid(file, voxel, pos);
					}
				}
			}
		}
	}

	fprintf(file, "\n}");

	Item* items = *getItems();
	int len = dynList_size(items);
	for (int i = 0; i < len; i++)
	{
		generateItem(file, &items[i]);
	}

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
