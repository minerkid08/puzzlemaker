#pragma once

typedef struct
{
	float moveSpeed;
	float boostSpeed;
	float fov;
	float rotSnap;
} EditorSettings;

extern EditorSettings editorSettings;

void loadEditorSettings();
void saveEditorSettings();
