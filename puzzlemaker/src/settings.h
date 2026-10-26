#pragma once

typedef struct
{
	float gizmoLen;
	float moveSpeed;
	float boostSpeed;
	float arrowSens;
	float orbitSens;
	char lockMouseOnPan;
	float panSens;
	float fov;
	float rotSnap;
} EditorSettings;

extern EditorSettings editorSettings;

void loadEditorSettings();
void saveEditorSettings();
