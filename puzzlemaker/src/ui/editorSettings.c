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

	igEnd();
}
