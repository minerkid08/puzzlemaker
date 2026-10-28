#include "compileThread.h"
#include <stdio.h>
#include <string.h>
#include "jsonUtils.h"
#include "mapsettings.h"

#ifdef __linux__
#include <unistd.h>
#else
#include <windows.h>
#endif

CompileStatus compileStatus;

void processString(const char* str, char* out)
{
	int len = strlen(str);
	char* ptr = out;

	for (int i = 0; i < len; i++)
	{
		if (str[i] != '%')
		{
			*ptr = str[i];
			ptr++;
			continue;
		}

		if (str[i + 1] == ' ')
		{
			*ptr = '%';
			ptr++;
			*ptr = ' ';
			ptr++;
			i++;
		}

		if (str[i + 1] == 'f')
		{
			int l = snprintf(ptr, out + 512 - ptr, "%s", mapSettings.name);
			ptr += l;
		}

		if (str[i + 1] == 'b')
		{
			int l = snprintf(ptr, out + 512 - ptr, "%s", compileStatus.bin);
			ptr += l;
		}

		if (str[i + 1] == 'p')
		{
			int l = snprintf(ptr, out + 512 - ptr, "%s", compileStatus.p2ce);
			ptr += l;
		}

		if (str[i + 1] == 'c')
		{
			int l = snprintf(ptr, out + 512 - ptr, "%s", compileStatus.curPath);
			ptr += l;
		}
		i++;
	}
	*ptr = 0;
}

void openCompilePopup();

void startCompile()
{
	printf("%s\n", mapSettings.name);
#ifdef __linux__
	getcwd(compileStatus.curPath, 512);
#else
	GetCurrentDirectory(512, compileStatus.curPath);
#endif
	compileStatus.failed = 0;
  compileStatus.currentStep = 0;
  openCompilePopup();
  resumeCompileThread();
}

void killCompileProcess();
void cancelCompile()
{
  killCompileProcess();
  compileStatus.failed = 1;
}

int getCompileStep()
{
	return compileStatus.currentStep;
}

char compileFailed()
{
	return compileStatus.failed;
}

int getCompileStepCount()
{
	return compileStatus.stepCount;
}

CompileStep* getCompileSteps()
{
	return compileStatus.compileSteps;
}
