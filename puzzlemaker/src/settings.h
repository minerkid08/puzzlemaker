#pragma once

typedef struct
{
	float moveSpeed;
	float boostSpeed;
	float fov;
} EditorSettings;

extern EditorSettings editorSettings;

void loadEditorSettings();
void saveEditorSettings();
