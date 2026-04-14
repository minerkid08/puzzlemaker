#include <linux/limits.h>
#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "compileThread.h"
#include <cimgui.h>

float frac = 0;

char open;
void openCompilePopup(const char* filename)
{
	open = 1;
	frac = 0;
	igOpenPopup_Str("compile", 0);
	startCompile(filename);
}

void updateCompilePopup()
{
	CompileStep* steps = getCompileSteps();
	ImVec2 zero = {0, 0};

	if (igBeginPopupModal("compile", (bool*)&open, 0))
	{
		char failed = compileFailed();;
		int step = getCompileStep();
		float count = getCompileStepCount();
		frac = step / (count - 1);
		igProgressBar(frac, zero, steps[step].name);
		if(failed)
			igText("compile step '%s' failed", steps[step].name);
		if (step != count && (failed == 0))
		{
			if (igButton("cancel", zero))
			{
				cancelCompile();
				igCloseCurrentPopup();
			}
		}
		else
		{
			if (igButton("done", zero))
				igCloseCurrentPopup();
		}
		igEndPopup();
	}
}
