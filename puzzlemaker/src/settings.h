#pragma once

typedef struct
{
	const char* theme;
	const char** themeList;
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
