#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "compile/compile.h"
#include "export/export.h"
#include <cimgui.h>

float frac = 0;

static char open = 0;
static char compileRunning = 0;
void openCompilePopup()
{
	open = 1;
	frac = 0;
	compileRunning = 1;
	igOpenPopup_Str("compile", 0);
}

void updateCompilePopup()
{
	CompileStep* steps = getCompileSteps();
	ImVec2 zero = {0, 0};

	if (open == 0 && compileRunning)
	{
		cancelCompile();
		compileRunning = 0;
	}

	if (igBeginPopupModal("compile", (bool*)&open, 0))
	{
		char failed = compileFailed();
		int step = getCompileStep();
		float count = getCompileStepCount();
		frac = step / (count - 1);
		igProgressBar(frac, zero, steps[step].name);
		if (failed)
			igText("compile step '%s' failed", steps[step].name);

		ImVec2 size = igGetContentRegionAvail();
		ImGuiStyle* style = igGetStyle();
		size.y -= style->FramePadding.y * 2 + style->FontSizeBase + style->ItemSpacing.y * 2;

		igBeginChild_Str("output", size, ImGuiChildFlags_Borders, 0);

		const char* message = getMessage();
		int len = strlen(message);
		const char* end = strchr(message, '\n');
		while (end)
		{
			igTextEx(message, end, 0);
			message = end + 1;
			end = strchr(message, '\n');
		}

		igTextEx(message, strchr(message, 0), 0);

		if (igGetScrollY() >= igGetScrollMaxY())
			igSetScrollHereY(1.0f);
		igEndChild();

		if (step != count && (failed == 0))
		{
      igBeginDisabled(step == 0);
			if (igButton("cancel", zero))
			{
				compileRunning = 0;
				cancelCompile();
				igCloseCurrentPopup();
			}
      igEndDisabled();
		}
		else
		{
			compileRunning = 0;
			if (igButton("done", zero))
				igCloseCurrentPopup();
		}
		if (igIsKeyPressed_Bool(ImGuiKey_Escape, 0))
		{
			if (compileRunning)
			{
				compileRunning = 0;
				cancelCompile();
			}
			igCloseCurrentPopup();
		}
		igEndPopup();
	}
}
