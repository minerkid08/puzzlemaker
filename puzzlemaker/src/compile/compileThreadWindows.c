#ifdef _WIN32
#include "compile/compileThread.h"
#include "dynList.h"
#include "export/export.h"
#include <io.h>
#include <stdio.h>
#include <windows.h>

extern CompileStatus compileStatus;

HANDLE event;

static char buf[512];
static char proc[1024];
static PROCESS_INFORMATION pi;
static char procRunning = 0;

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

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = 1;

	STARTUPINFO si;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	HANDLE hr;
	HANDLE hw;

	CreatePipe(&hr, &hw, &sa, 0);
	SetHandleInformation(hr, HANDLE_FLAG_INHERIT, 0);
	si.hStdInput = 0;
	si.hStdOutput = hw;
	si.hStdError = hw;
	si.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcess(0, proc, 0, 0, TRUE, 0, 0, compileStatus.workingDir, &si, &pi))
	{
		CloseHandle(hw);
		CloseHandle(hr);
		writeMessagef("CreateProcess failed (%d).\n", GetLastError());
		writeMessagef("command line: '%s'\n", proc);
		writeMessagef("working dir: '%s'\n", compileStatus.workingDir);

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
			writeMessage("bad command line\n");

		if (_access(compileStatus.workingDir, 0))
			writeMessage("bad working dir\n");
		return 1;
	}
  procRunning = 1;
	CloseHandle(hw);

	char buf[512];
	unsigned long int len2 = 1;
	while (len2 > 0)
	{
		if (!ReadFile(hr, buf, 512, &len2, 0))
			break;
		writeMessagen(buf, len2);
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
  procRunning = 0;
	CloseHandle(hr);

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
		clearMessage();
		for (int i = 0; i < compileStatus.stepCount; i++)
		{
			CompileStep* step = &compileStatus.compileSteps[i];
			writeMessagef("running %s\n\n", step->name);
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

void killCompileProcess()
{
	if (procRunning)
  {
    procRunning = 0;
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
}

#endif
