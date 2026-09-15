#pragma once
#include "compile/compile.h"

typedef struct
{
	int stepCount;
	int currentStep;
	CompileStep* compileSteps;
	const char* p2ce;
	const char* bin;
	char curPath[512];
	char buf[256];
  char workingDir[512];
	char failed;
} CompileStatus;

void startCompileThread();
void loadTaskList();

void resumeCompileThread();
