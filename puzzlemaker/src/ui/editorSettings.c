#include "camera.h"
#include "settings.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"

static bool open = 0;

void openEditorSettingsUi()
{
	open = 1;
}

void renderEditorSettingsUi()
{
	if (open == 0)
		return;

	igBegin("Editor Settings", &open, ImGuiWindowFlags_NoDocking);
  
	igInputFloat("move speed", &editorSettings.moveSpeed, 1, 1, "%.2f", 0);
	igInputFloat("boost speed", &editorSettings.boostSpeed, 1, 1, "%.2f", 0);
	if(igInputFloat("fov", &editorSettings.fov, 1, 1, "%.2f", 0))
	{
		initCamera();
	}

	igEnd();
}
