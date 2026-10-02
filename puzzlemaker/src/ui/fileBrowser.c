#include "ui/fileBrowser.h"
#include "dynList.h"
#include "mapsettings.h"
#include "save.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"

static char path[256];
static char filename[64];

static char mode = 0;
static bool open = 0;

static char** directories = 0;
static char** files = 0;

static float textboxWidth = 0;
static __attribute__((constructor)) void init()
{
	path[0] = 0;
}

void scanDir()
{
	textboxWidth = 0;

	textboxWidth += igCalcTextSize("mkdir", 0, 0, -1).x;
	if (mode == MODE_SAVE)
		textboxWidth += igCalcTextSize("save", 0, 0, -1).x;
	else
		textboxWidth += igCalcTextSize("open", 0, 0, -1).x;

	ImGuiStyle* style = igGetStyle();

	textboxWidth += style->FramePadding.x * 4;
	textboxWidth += style->ItemSpacing.x * 2.0f;

	if (directories == 0)
	{
		directories = dynList_new(0, sizeof(char*));
		dynList_reserve((void**)&directories, 16);
		files = dynList_new(0, sizeof(char*));
		dynList_reserve((void**)&files, 16);
	}
	else
	{
		for (int i = 0; i < dynList_size(directories); i++)
			free(directories[i]);
		dynList_resize((void**)&directories, 0);

		for (int i = 0; i < dynList_size(files); i++)
			free(files[i]);
		dynList_resize((void**)&files, 0);
	}
	listFiles(path, (const char***)&directories, (const char***)&files, ".chamb");
}

void fileBrowserOpen(char mode2)
{
	mode = mode2;
	open = 1;
	strcpy(path, "maps");
	scanDir();
}

void fileBrowserRender()
{
	if (open == 0)
		return;
	igBegin("File Browser", &open, ImGuiWindowFlags_NoDocking);

	float buttonSize = igGetStyle()->FontScaleMain * 30 + igGetStyle()->FramePadding.y * 4;
	ImVec2 size = igGetContentRegionAvail();

	size.y = size.y - buttonSize;

	ImVec2 zero = {0.0f, 0.0f};

	if (igButton("^", zero))
	{
		int l = strlen(path);
		int slashPos = 0;
		for (int i = 0; i < l; i++)
		{
			if (path[i] == '/')
				slashPos = i;
		}
		if (slashPos != 0)
		{
			path[slashPos] = 0;
			scanDir();
		}
	}
	igSameLine(0, -1);
	igText(path);

	int i = 0;
	if (igBeginListBox("##list", size))
	{
		ImVec4 color = {0.5f, 0.5f, 1.0f, 1.0f};
		igPushStyleColor_Vec4(ImGuiCol_Text, color);
		int l = dynList_size(directories);
		for (int j = 0; j < l; j++)
		{
			const char* dir = directories[j];
			igPushID_Int(i++);
			if (igSelectable_Bool(dir, 0, 0, zero))
			{
				strncat(path, "/", 255);
				strncat(path, dir, 255);
				scanDir();
			}
			igPopID();
		}
		igPopStyleColor(1);

		l = dynList_size(files);
		for (int j = 0; j < l; j++)
		{
			const char* file = files[j];
			igPushID_Int(i++);

			if (igSelectable_Bool(file, 0, 0, zero))
			{
				strncat(path, "/", 255);
				strncat(path, file, 255);

				int l = strlen(path);
				int dotPos = 0;
				for (int i = 0; i < l; i++)
				{
					if (path[i] == '.')
						dotPos = i;
				}
				if (dotPos != 0)
				{
					path[dotPos] = 0;
				}

				strcpy(mapSettings.name, path + 5);
				if (mode == MODE_SAVE)
					save();
				if (mode == MODE_LOAD)
					load();
				open = 0;
			}
			igPopID();
		}
		igEndListBox();
	}

	igPushItemWidth(igGetContentRegionAvail().x - textboxWidth);
	igInputText("##filename", filename, 64, 0, 0, 0);
	igPopItemWidth();

	igSameLine(0, -1);
	if (igButton("mkdir", zero))
	{
		strncat(path, "/", 255);
		strncat(path, filename, 255);
		makeDir(path);
		scanDir();
	}
	igSameLine(0, -1);
	if (mode == MODE_LOAD)
	{
		if (igButton("open", zero))
		{
			strncat(path, "/", 255);
			strncat(path, filename, 255);
			strcpy(mapSettings.name, path + 5);
			load();
			open = 0;
		}
	}
	else
	{
		if (igButton("save", zero))
		{
			strncat(path, "/", 255);
			strncat(path, filename, 255);
			strcpy(mapSettings.name, path + 5);
			save();
			open = 0;
		}
	}

	igEnd();
}

void fileBrowserSave()
{
	if (path[0] == 0)
	{
		fileBrowserOpen(MODE_SAVE);
		return;
	}
	strcpy(mapSettings.name, path + 5);
	save();
}

char* fileBrowserGetPath()
{
	return path;
}
