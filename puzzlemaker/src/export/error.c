#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef __linux__
#include <unistd.h>
#else
#include <windows.h>
#endif

#define BUFSIZE 65536

#ifdef _WIN32
static HANDLE out;

static __attribute((constructor)) void init()
{
  out = GetStdHandle(STD_OUTPUT_HANDLE);
}

#endif

char message[BUFSIZE];
int size = BUFSIZE;
char* start = message;

void clearMessage()
{
  message[0] = 0;
  start = message;
  size = BUFSIZE;
}

char* getMessage()
{
  return message;
}

void writeMessage(const char* data)
{
  strncpy(start, data, size);
  int len = strlen(data);
#ifdef __linux__
  write(STDOUT_FILENO, data, len);
#else
  WriteFile(out, data, len, 0, 0);
#endif
  size -= len;
  start += len;
}

void writeMessagen(const char* data, int len)
{
#ifdef __linux__
  write(STDOUT_FILENO, data, len);
#else
  WriteFile(out, data, len, 0, 0);
#endif
  memcpy(start, data, len);
  size -= len;
  start += len;
  start[0] = 0;
}

void writeMessagef(const char* fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  int len = vsnprintf(start, size, fmt, va);
#ifdef __linux__
  write(STDOUT_FILENO, start, len);
#else
  WriteFile(out, start, len, 0, 0);
#endif
  start += len;
  size -= len;
}
