#include "camera.h"
#include "dynList.h"
#include "settings.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"

static bool open = 0;
extern ImVec2 zero;
extern char queueThemeUpdate;

void openEditorSettingsUi()
{
	open = 1;
}

void renderEditorSettingsUi()
{
	if (open == 0)
		return;

	igBegin("Editor Settings", &open, ImGuiWindowFlags_NoDocking);

	igInputFloat("Move Speed", &editorSettings.moveSpeed, 0.25, 1, "%.2f", 0);
	igInputFloat("Boost Speed", &editorSettings.boostSpeed, 0.25, 1, "%.2f", 0);
	igInputFloat("Look Sensitivity (arrow)", &editorSettings.arrowSens, 0.25, 1, "%.2f", 0);
	igInputFloat("Orbit Sensitivity", &editorSettings.orbitSens, 0.25, 1, "%.2f", 0);
	igInputFloat("Pan Sensitivity", &editorSettings.panSens, 0.25, 1, "%.2f", 0);
	igCheckbox("Capture Mouse on Pan", (bool*)&editorSettings.lockMouseOnPan);
	if (igInputFloat("Fov", &editorSettings.fov, 1, 1, "%.2f", 0))
		initCamera();
	igInputFloat("Rotation Snap", &editorSettings.rotSnap, 1, 1, "%.2f", 0);
	igInputFloat("Angle Gizmo Length", &editorSettings.gizmoLen, 0.25f, 1, "%.2f", 0);

	if (igBeginCombo("Editor Theme", editorSettings.theme, 0))
	{
		int len = dynList_size(editorSettings.themeList);
		for (int i = 0; i < len; i++)
		{
			char selected = (editorSettings.themeList[i] == editorSettings.theme);
			if (igSelectable_Bool(editorSettings.themeList[i], selected, 0, zero))
				editorSettings.theme = editorSettings.themeList[i];
			if (selected)
				igSetItemDefaultFocus();
		}
		igEndCombo();
	}
	igSameLine(0, -1);
	igTextDisabled("(Requires Restart)");

	igEnd();
}
