#pragma once

char initErrorStack(const char* initalFrame, void(*func)());
void pushStackFrame(const char* name);
void popStackFrame();
void errorf(const char* name, ...);
void errorf2(const char* name, ...);
const char* getError();
void setError(const char* err);
const char** getStackTrace();
void clearError();

void startErrorLoop();
void openErrorPopup();
