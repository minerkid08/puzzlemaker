#include "dynList.h"
#include "itemPanel.h"
#include "ui.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdlib.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include <cimgui.h>

const char** stackTrace = 0;
const char* err = 0;

char errorState = 0;

static char open = 0;
static char wasOpen = 0;
extern GLFWwindow* window;

void startErrorLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		uiNewFrame();
		uiMenuBar();
		uiEndFrame();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	exit(1);
}

void openErrorPopup()
{
	open = 1;
	igOpenPopup_Str("error", 0);
}

void renderErrorPopup()
{
	ImVec2 zero = {0, 0};

	if (open == 0 && wasOpen)
		exit(1);

	if (errorState && open == 0)
	{
		open = 1;
		igOpenPopup_Str("error", 0);
	}

	if (open)
		wasOpen = 1;

	if (igBeginPopupModal("error", (bool*)&open, 0))
	{
		if (err)
			igText("error: %s", err);
		if (stackTrace)
		{
			int len = dynList_size(stackTrace);
			for (int i = 0; i < len; i++)
				igText("in %s", stackTrace[i]);
		}
		igEndPopup();
	}
}
