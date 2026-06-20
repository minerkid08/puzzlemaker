#include "ui/fileBrowser.h"
#include "dynList.h"
#include "save.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"

static char path[256];
static char filename[64];

static char mode = 0;
static bool open = 0;

static char** dirs = 0;
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

	if (dirs == 0)
	{
		dirs = dynList_new(0, sizeof(char*));
		dynList_reserve((void**)&dirs, 16);
		files = dynList_new(0, sizeof(char*));
		dynList_reserve((void**)&files, 16);
	}
	else
	{
		for (int i = 0; i < dynList_size(dirs); i++)
			free(dirs[i]);
		dynList_resize((void**)&dirs, 0);

		for (int i = 0; i < dynList_size(files); i++)
			free(files[i]);
		dynList_resize((void**)&files, 0);
	}

	struct dirent* en;

	DIR* dir = opendir(path);
	if (dir)
	{
		while ((en = readdir(dir)) != 0)
		{
			if (en->d_type == DT_DIR)
			{
				if (strcmp(en->d_name, ".") == 0)
					continue;
				if (strcmp(en->d_name, "..") == 0)
					continue;
				int len = dynList_size(dirs);
				dynList_resize((void**)&dirs, len + 1);
				dirs[len] = strdup(en->d_name);
			}

			int len = strlen(en->d_name);
			if (strcmp(en->d_name + len - 6, ".chamb") == 0)
			{
				int l = dynList_size(files);
				dynList_resize((void**)&files, l + 1);
				files[l] = strdup(en->d_name);
			}
		}
		closedir(dir);
	}
	else
		printf("bad directory '%s'\n", path);
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
		ImVec4 color = {0.0f, 0.0f, 1.0f, 1.0f};
		igPushStyleColor_Vec4(ImGuiCol_Text, color);
		int l = dynList_size(dirs);
		for (int j = 0; j < l; j++)
		{
			const char* dir = dirs[j];
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

				if (mode == MODE_SAVE)
					save(path);
				if (mode == MODE_LOAD)
					load(path);
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
		mkdir(path, 0777);
		scanDir();
	}
	igSameLine(0, -1);
	if (mode == MODE_LOAD)
	{
		if (igButton("open", zero))
		{
			strncat(path, "/", 255);
			strncat(path, filename, 255);
			load(path);
			open = 0;
		}
	}
	else
	{
		if (igButton("save", zero))
		{
			strncat(path, "/", 255);
			strncat(path, filename, 255);
			save(path);
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
	save(path);
}

char* fileBrowserGetPath()
{
	return path;
}
