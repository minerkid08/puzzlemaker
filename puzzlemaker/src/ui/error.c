#include "error.h"
#include "dynList.h"
#include "itemPanel.h"
#include "ui.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdlib.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include <cimgui.h>

static char open = 0;
static char wasOpen = 0;
static char queueErrorPopup = 0;
extern GLFWwindow* window;
extern char running;
extern char metadataLoaded;

void startErrorLoop()
{
	printf("err: %s\n", getError());
	if (running)
		return;
	openErrorPopup();
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
	queueErrorPopup = 1;
}

void renderErrorPopup()
{
	ImVec2 zero = {0, 0};

	if (open == 0 && wasOpen)
	{
		wasOpen = 0;
		if (running == 0)
			exit(1);
    clearError();
	}

	if (open)
		wasOpen = 1;

	if (queueErrorPopup)
  {
    queueErrorPopup = 0;
    open = 1;
    igOpenPopup_Str("error", 0);
  }
		

	if (igBeginPopupModal("error", (bool*)&open, 0))
	{
    const char* errorMsg = getError();
    const char** stackTrace = getStackTrace();
		if (errorMsg)
			igText("error: %s", errorMsg);
		if (stackTrace)
		{
			int len = dynList_size(stackTrace);
			for (int i = 0; i < len; i++)
				igText("in %s", stackTrace[i]);
		}
		if (metadataLoaded)
		{
			if (igButton("package manager", zero))
				openPackageManager();
		}
		igEndPopup();
	}
}
