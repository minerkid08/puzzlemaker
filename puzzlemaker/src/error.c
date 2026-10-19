#include "dynList.h"
#include "setjmp.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static jmp_buf jmpPoint;

static const char** stackTrace;
static const char* error = 0;

static __attribute__((constructor)) void init()
{
	stackTrace = dynList_new(0, sizeof(const char*));
}

char initErrorStack(const char* initalFrame, void(*func)())
{
	int len = dynList_size(stackTrace);
	for (int i = 0; i < len; i++)
		free((char*)stackTrace[i]);
	dynList_resize((void**)&stackTrace, 1);
	stackTrace[0] = strdup(initalFrame);

	int res = setjmp(jmpPoint);
  if(res == 0)
  {
    func();
    return 0;
  }
  else
    return 1;
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
  longjmp(jmpPoint, 1);
}

void errorf2(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
  error = vstrdupf(fmt, va);
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
  if(error)
    free((char*)error);
  error = 0;
}
