#pragma once

typedef struct
{
	const char* name;
	const char* cmd;
} CompileStep;

void startCompile();
void startCompileThread();

int getCompileStep();
int getCompileStepCount();
CompileStep* getCompileSteps();

void cancelCompile();
char compileFailed();
