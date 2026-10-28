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
static char* file;

static char mode = 0;
static char queuePopup = 0;
static bool open = 0;
static bool loadPopup = 0;

static char** directories = 0;
static char** files = 0;

static float textboxWidth = 0;

extern char queueCompile;

#ifdef __linux__
__attribute__((constructor)) static void fileBrowserInit()
#else
void fileBrowserInit()
#endif
{
	path[0] = 0;
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*fileBrowserInitPtr)(void) = fileBrowserInit;

#pragma comment(linker, "/include:fileBrowserInitPtr")
#endif

void scanDir()
{
	textboxWidth = 0;

	ImGuiStyle* style = igGetStyle();

	if (mode & MODE_SAVE)
	{
		textboxWidth += style->FramePadding.x * 4;
		textboxWidth += style->ItemSpacing.x * 2.0f;
		textboxWidth += igCalcTextSize("Make Directory", 0, 0, -1).x;
		textboxWidth += igCalcTextSize("Save", 0, 0, -1).x;
	}
	else
	{
		textboxWidth += style->FramePadding.x * 2;
		textboxWidth += style->ItemSpacing.x * 1.0f;
		textboxWidth += igCalcTextSize("Open", 0, 0, -1).x;
	}

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
	if (mode & MODE_LOAD)
		loadPopup = 1;
	else
		open = 1;
	strcpy(path, "maps");
	scanDir();
}

static ImVec2 zero = {0.0f, 0.0f};

void fileBrowserRender()
{
	if (loadPopup)
	{
		loadPopup = 0;
		igOpenPopup_Str("Load", 0);
	}
	if (igBeginPopupModal("Load", 0, 0))
	{
		igText("Your current map might not be saved");
		igText("Would you like to save it?");

		if (igButton("Save", zero))
		{
			fileBrowserSave();
			igCloseCurrentPopup();
		}
		igSameLine(0, -1);
		if (igButton("Save As", zero))
		{
			fileBrowserOpen(MODE_SAVE);
			igCloseCurrentPopup();
		}
		igSameLine(0, -1);
		if (igButton("Don't Save", zero))
		{
			mode = MODE_LOAD;
			open = 1;
			igCloseCurrentPopup();
		}
		igSameLine(0, -1);
		if (igButton("Cancel", zero))
			igCloseCurrentPopup();

		igEndPopup();
	}

	if (open == 0)
		return;
	igBegin("File Browser", &open, ImGuiWindowFlags_NoDocking);

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

	ImGuiStyle* style = igGetStyle();
	float height = style->FramePadding.y * 2 + style->FontSizeBase + style->ItemSpacing.y * 2;
	ImVec2 size = igGetContentRegionAvail();

	size.y -= height;

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
			char* f = files[j];
			igPushID_Int(i++);

			if (igSelectable_Bool(f, 0, 0, zero))
			{
				file = f;
				if (mode & MODE_LOAD)
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
					load();
					open = 0;
				}
				else
					queuePopup = 1;
			}
			igPopID();
		}
		igEndListBox();
	}

	igPushItemWidth(igGetContentRegionAvail().x - textboxWidth);
	igInputText("##filename", filename, 64, 0, 0, 0);
	igPopItemWidth();

	igSameLine(0, -1);
	if (mode & MODE_LOAD)
	{
		if (igButton("Open", zero))
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
		if (igButton("Make Directory", zero))
		{
			strncat(path, "/", 255);
			strncat(path, filename, 255);
			makeDir(path);
			scanDir();
		}
		igSameLine(0, -1);
		if (igButton("Save", zero))
		{
			strncat(path, "/", 255);
			strncat(path, filename, 255);
			strcpy(mapSettings.name, path + 5);
			save();
			if (mode & QUEUE_COMPILE)
				queueCompile = 1;
			open = 0;
		}
	}

	igEnd();

	if (queuePopup)
	{
		queuePopup = 0;
		if (mode & MODE_SAVE)
			igOpenPopup_Str("Save As", 0);
	}
	if (igBeginPopupModal("Save As", 0, 0))
	{
		igText("You are going to overwrite %s/%s", path, file);
		igText("Are you sure?");
		if (igButton("Yes", zero))
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
			save();
			if (mode & QUEUE_COMPILE)
				queueCompile = 1;
			open = 0;
			igCloseCurrentPopup();
		}
		igSameLine(0, -1);
		if (igButton("No", zero))
			igCloseCurrentPopup();
		igEndPopup();
	}
}

void fileBrowserSave()
{
	if (path[5] == 0)
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
