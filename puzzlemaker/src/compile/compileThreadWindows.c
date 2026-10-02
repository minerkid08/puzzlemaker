#ifdef _WIN32
#include "compile/compileThread.h"
#include "dynList.h"
#include <io.h>
#include <stdio.h>
#include <windows.h>

extern CompileStatus compileStatus;

HANDLE event;

static char buf[512];
static char proc[1024];

void processString(const char* str, char* out);

int runCmd(char* cmd)
{
	processString(cmd, buf);
  memset(proc, 0, 1024);

	int len = strlen(buf);
	int j = 1;
  proc[0] = '"';
	for (int i = 0; i < len; i++)
	{
		if (buf[i] == '%')
		{
			proc[j] = buf[i + 1];
			continue;
		}
		if (buf[i] == ' ' && proc[j] != ' ')
		{
      proc[j] = '"';
      proc[j + 1] = ' ';
      proc[j + 2] = '"';
			j += 3;
			continue;
		}
		proc[j] = buf[i];
		j++;
	}
  proc[j] = '"';
  proc[j + 1] = 0;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	if (!CreateProcess(0, proc, 0, 0, FALSE, 0, 0, compileStatus.workingDir, &si, &pi))
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		printf("command line: '%s'\n", proc);
		printf("working dir: '%s'\n", compileStatus.workingDir);

		int len = strlen(buf);
		for (int i = 0; i < len; i++)
		{
			if (buf[i] == '\\')
				continue;
			if (buf[i] == ' ')
			{
				buf[i] = 0;
				break;
			}
		}
		if (_access(buf, 0))
			printf("bad command line\n");

		if (_access(compileStatus.workingDir, 0))
			printf("bad working dir\n");
		return 1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	if (exitCode)
	{
		printf("command line: '%s'\n", proc);
		printf("working dir: '%s'\n", compileStatus.workingDir);
		compileStatus.failed = 1;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}

void resumeCompileThread()
{
	SetEvent(event);
}

int exportMap();

DWORD WINAPI compileThread(LPVOID data)
{
	while (1)
	{
		printf("waiting for compile\n");
		WaitForSingleObject(event, INFINITE);
		printf("compile started\n");
		compileStatus.currentStep = 0;
		ResetEvent(event);
		for (int i = 0; i < compileStatus.stepCount; i++)
		{
			CompileStep* step = &compileStatus.compileSteps[i];
			if (i == 0)
			{
				if (exportMap())
					compileStatus.failed = 1;
			}
			else
			{
				if (runCmd((char*)step->cmd))
					compileStatus.failed = 1;
			}
			if (compileStatus.failed || compileStatus.currentStep == -1)
				break;
			compileStatus.currentStep++;
		}
	}

	return 0;
}

void startCompileThread()
{
	event = CreateEvent(0, 1, 0, 0);
	int data = 100;
	DWORD threadId;

	HANDLE thread = CreateThread(0, 0, compileThread, &data, 0, &threadId);

	if (thread == NULL)
	{
		printf("CreateThread failed (%d)\n", GetLastError());
		return;
	}
	CloseHandle(thread);

	loadTaskList();
}

#endif
