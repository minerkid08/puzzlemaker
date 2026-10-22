#include "cjson.h"
#include "dynList.h"
#include "packages/package.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"
#include <stdbool.h>

static bool open;
#ifdef __linux__
#include <unistd.h>
#else
#include <windows.h>
#endif

extern const char* commandLine;
extern GLFWwindow* window;
extern Package* packageList;

void openPackageManager()
{
	open = !open;
}

static ImVec2 uv0 = {.x = 0, .y = 1};
static ImVec2 uv1 = {.x = 1, .y = 0};

void packageManagerRender()
{
	if (open == 0)
		return;
	igBegin("Package Manager", &open, ImGuiWindowFlags_NoDocking);

	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;

	ImGuiStyle* style = igGetStyle();
	float height = style->FramePadding.y * 2 + style->FontSizeBase + style->ItemSpacing.y * 2;
	ImVec2 size = {0, -height};

	igBeginChild_Str("spacer", size, 0, 0);
	int packageCount = dynList_size(packageList);
	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		if (igTreeNodeEx_Str(package->baseName, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImVec2 size;
			size.x = 128;
			size.y = 128;
			ImTextureRef_c tex;
			tex._TexID = package->icon;
			tex._TexData = 0;
			igImage(tex, size, uv0, uv1);
			igSameLine(0, -1);
			size.x = igGetContentRegionAvail().x - 150;
			igBeginChild_Str("text", size, ImGuiChildFlags_Borders, 0);

			igText("by %s", package->author);
			igCheckbox("enabled", (bool*)&package->enabled);
			igSeparator();
			igTextWrapped("%s", package->description);
			igEndChild();
			igSameLine(0, -1);

			size.x = igGetContentRegionAvail().x;
			igBeginChild_Str("text2", size, ImGuiChildFlags_Borders, 0);
			igText("v%s", package->version);
			if (package->requiredAddons)
			{
				int s = dynList_size(package->requiredAddons);
				igText("p2ce addons:");
				for (int j = 0; j < s; j++)
					igText(package->requiredAddons[j]);
			}

			igEndChild();
			igTreePop();
		}
	}
	igEndChild();

	if (igButton("apply", zero))
	{
		cJSON* json = cJSON_CreateObject();
		for (int i = 0; i < packageCount; i++)
		{
			Package* package = &packageList[i];
			cJSON_AddBoolToObject(json, package->name, package->enabled);
		}

		char* str = cJSON_Print(json);

		FILE* file = fopen("package.json", "wb");
		fwrite(str, strlen(str), 1, file);
		fclose(file);

		free(str);
		cJSON_Delete(json);

		glfwDestroyWindow(window);
		glfwTerminate();

#ifdef __linux__
		char* args[2];
		args[0] = (char*)commandLine;
		args[1] = 0;
		if (execvp(args[0], args))
		{
			perror("execvp");
			exit(1);
		}
#else
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		memset(&pi, 0, sizeof(pi));

		CreateProcess(0, (char*)commandLine, 0, 0, FALSE, 0, 0, 0, &si, &pi);
		exit(1);
#endif
	}
	igEnd();
}
