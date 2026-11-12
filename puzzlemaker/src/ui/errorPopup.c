#include "compile/compileThread.h"
#include "dynList.h"
#include "error.h"
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
static char p2ceOpen = 0;
static char p2ceWasOpen = 0;
static char p2ceQueuePopup = 0;
static char* p2ce;
extern GLFWwindow* window;
extern char running;
extern CompileStatus compileStatus;
extern char metadataLoaded;
char errorActive = 0;

void startErrorLoop()
{
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
	errorActive = 1;
	queueErrorPopup = 1;
}

void openP2ceErrorPopup()
{
	p2ce = strdup(compileStatus.p2ce);
	int j = 0;
	for (int k = 0; k < strlen(p2ce); k++)
	{
		if (p2ce[k] == '%')
			continue;
		p2ce[j++] = p2ce[k];
	}
	p2ce[j] = 0;
	p2ceQueuePopup = 1;
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
		errorActive = 0;
	}

	if (p2ceOpen == 0 && p2ceWasOpen)
	{
		p2ceWasOpen= 0;
		free(p2ce);
	}

	if (open)
		wasOpen = 1;
	if (p2ceOpen)
		p2ceWasOpen = 1;

	if (queueErrorPopup)
	{
		queueErrorPopup = 0;
		open = 1;
		igOpenPopup_Str("Error", 0);
	}

	if (p2ceQueuePopup)
	{
		p2ceQueuePopup = 0;
		p2ceOpen = 1;
		igOpenPopup_Str("P2CE Not Found", 0);
	}

	if (igBeginPopupModal("Error", (bool*)&open, 0))
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
			if (igButton("Package Manager", zero))
				openPackageManager();
			if (igIsKeyPressed_Bool(ImGuiKey_Escape, 0))
				igCloseCurrentPopup();
			packageManagerRender();
		}
		igEndPopup();
	}

	if (igBeginPopupModal("P2CE Not Found", (bool*)&p2ceOpen, 0))
	{
		igText("Your Portal 2 Community Edition installation was not found at");
		igText("'%s'", p2ce);
		igText("Double check that the p2ce field in compileSteps.json points to your P2CE installation");
		igText("Instance management and map compillation have been disabled for this session");
		if (igIsKeyPressed_Bool(ImGuiKey_Escape, 0))
			igCloseCurrentPopup();
		igEndPopup();
	}
}
