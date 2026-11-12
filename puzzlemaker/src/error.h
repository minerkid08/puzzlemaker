#pragma once

#include "setjmp.h"
#include <string.h>

extern jmp_buf jmpPoint;

#define initErrorStack(initalFrame, res)                                                                               \
	_initErrorStack(initalFrame);                                                                                      \
	res = setjmp(jmpPoint)

#define initErrorStackD(initalFrame, res)                                                                               \
	_initErrorStack(strdup(initalFrame));                                                                                      \
	res = setjmp(jmpPoint)

void _initErrorStack(const char* initalFrame);
void pushStackFrame(const char* name);
void popStackFrame();
void errorf(const char* name, ...);
const char* getError();
void setError(const char* err);
const char** getStackTrace();
void clearError();
void endStackTrace();

void startErrorLoop();
void openErrorPopup();
