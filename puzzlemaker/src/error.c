#include "error.h"
#include "dynList.h"
#include "setjmp.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

jmp_buf jmpPoint;
static char jmpActive = 0;

static const char** stackTrace;
static const char* error = 0;

#ifdef __linux__
__attribute__((constructor)) static void errorInit()
#else
void errorInit()
#endif
{
	stackTrace = dynList_new(0, sizeof(const char*));
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*errorInitPtr)(void) = errorInit;

#pragma comment(linker, "/include:errorInitPtr")
#endif

void _initErrorStack(const char* initalFrame)
{
	int len = dynList_size(stackTrace);
	for (int i = 0; i < len; i++)
		free((char*)stackTrace[i]);
	dynList_resize((void**)&stackTrace, 1);
	stackTrace[0] = initalFrame;
	jmpActive = 1;
}

void pushStackFrame(const char* name)
{
	int len = dynList_size(stackTrace);
	dynList_resize((void**)&stackTrace, len + 1);
	stackTrace[len] = strdup(name);
}

void popStackFrame()
{
	int len = dynList_size(stackTrace);
	free((char*)stackTrace[len - 1]);
	dynList_resize((void**)&stackTrace, len - 1);
}

void errorf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	error = vstrdupf(fmt, va);
	printf("error: %s\n", error);
	int len = dynList_size(stackTrace);
	for (int i = 0; i < len; i++)
		printf("in %s\n", stackTrace[i]);
	if (jmpActive)
	{
		jmpActive = 0;
		longjmp(jmpPoint, 1);
	}
	else
		openErrorPopup();
}

const char* getError()
{
	return error;
}

void setError(const char* err)
{
	error = strdup(err);
}

const char** getStackTrace()
{
	return stackTrace;
}

void clearError()
{
	if (error)
		free((char*)error);
	error = 0;
	jmpActive = 0;
}

void endStackTrace()
{
	jmpActive = 0;
	int len = dynList_size(stackTrace);
	for (int i = 0; i < len; i++)
		free((char*)stackTrace[i]);
	dynList_resize((void**)&stackTrace, 0);
}
