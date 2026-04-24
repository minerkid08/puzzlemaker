#ifdef WINDOWS
#include "compile/compileThread.h"
#include "dynList.h"

CompileStep* steps;

void startCompile(const char* filename)
{
}
void startCompileThread()
{
	steps = dynList_new(1, sizeof(CompileStep));
	steps[0].cmd = "echo no";
	steps[0].name = "error";
}
int getCompileStep()
{
	return 0;
}
int getCompileStepCount()
{
	return 1;
}

CompileStep* getCompileSteps()
{
	return steps;
}
void cancelCompile()
{
}
char compileFailed()
{
	return 1;
}

#endif
