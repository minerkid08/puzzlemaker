#pragma once

#define MODE_SAVE 1
#define MODE_LOAD 2

void fileBrowserOpen(char mode);
void fileBrowserRender();
void fileBrowserSave();
char* fileBrowserGetPath();
