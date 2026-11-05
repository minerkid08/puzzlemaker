#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern ImVec2 zero;

static bool open = 0;

void openKeybindsPanel()
{
	open = 1;
}

#define bind(key, tag)                                                                                                 \
	igButton(key, buttonSize);                                                                                         \
	igSameLine(0, -1);                                                                                                 \
	igText(tag);

void renderKeybinds()
{
	if (open == 0)
		return;

	igBegin("Keybinds", &open, 0);

	ImVec2 buttonSize = {96, 0};

	igText("Keybinds are currently not configurable");

	igBeginDisabled(1);

	igSeparatorText("Camera Movement");
	bind("w", "forward");
	bind("s", "backward");
	bind("a", "left");
	bind("d", "right");
	bind("arrowUp", "lookUp");
	bind("arrowDown", "lookDown");
	bind("arrowLeft", "lookLeft");
	bind("arrowRight", "lookRight");
	bind("mouse2", "orbit");
	bind("mouse3", "pan");

	igSeparatorText("Voxels");
	bind("2", "push");
	bind("3", "pull");
	bind("r", "toggle portalibility");
	bind("z", "toggle texture (64 unit texture and 32 unit texture)");

	igSeparatorText("Items and Antlines");
	bind("c", "open item selection menu");
	bind("g", "grab");
	bind("f", "grab for rotating");
	bind("q", "rotate 90");
	bind("e", "rotate -90");
	bind("h", "hide");
	bind("alt+h", "show");
	bind("ctrl+c", "copy item");
	bind("crtl+v", "paste item");
	bind("crtl+x", "cut item");
	bind("2##2", "extend antline by 1");
	bind("3##3", "shrink antline by 1");
	bind("z##4", "add antline segment");

	igSeparatorText("Other");
	bind("mouse1", "select item, voxel or antline");
	bind("ctrl+s", "save map");
	bind("f9", "compile map");

	igEndDisabled();

	igEnd();
}
