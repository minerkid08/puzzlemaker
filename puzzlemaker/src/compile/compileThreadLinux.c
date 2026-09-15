#ifdef linux

#include "compileThread.h"
#include "dynList.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void processString(const char* str, char* out);

#define pipeRead pipefd[0]
#define pipeWrite pipefd[1]

static int pipefd[2];
static pthread_t thread;

static char buf[512];

extern CompileStatus compileStatus;

int runCmd(char* cmd)
{
	processString(cmd, buf);

	pid_t pid;

	pid = fork();

	if (pid < 0)
	{
		perror("fork");
		return 1;
	}
	else if (pid == 0)
	{
		// Child process

		int arglen = strlen(buf);

		char** data = dynList_new(1, sizeof(char*));

		data[0] = buf;
		int j = 0;
		for (int i = 0; i < arglen; i++)
		{
			buf[j] = buf[i];
			if (buf[i] == '%')
			{
				buf[j] = ' ';
				i++;
				j++;
				continue;
			}
			if (buf[i] == ' ')
			{
				buf[j] = '\0';
				int l = dynList_size(data);
				dynList_resize((void**)&data, l + 1);
				data[l] = buf + j + 1;
			}
			j++;
		}
		buf[j] = '\0';

		int l = dynList_size(data);
		dynList_resize((void**)&data, l + 1);
		data[l] = 0;


		if (chdir(compileStatus.workingDir))
			perror("chdir");

		execvp(data[0], data);
		perror("execlp");
		return 1;
	}
	else
	{
		// Parent process
		int status = 0;
		waitpid(pid, &status, 0);
		if (status)
			compileStatus.failed = 1;
	}
	return 0;
}

int exportMap();

void* compileThread(void* e)
{
	while (1)
	{
		char c;
		read(pipeRead, &c, 1);
		compileStatus.currentStep = 0;
		for (int i = 0; i < compileStatus.stepCount; i++)
		{
			CompileStep* step = &compileStatus.compileSteps[i];
			if (i == 0)
			{
				if (exportMap())
					compileStatus.failed = 1;
			}
			else
				runCmd((char*)step->cmd);
			if (compileStatus.failed || compileStatus.currentStep == -1)
				break;
			compileStatus.currentStep++;
		}
	}
	return 0;
}

void resumeCompileThread()
{
	char c = 1;
	write(pipeWrite, &c, 1);
}

void startCompileThread()
{
	pipe(pipefd);
	pthread_create(&thread, 0, compileThread, 0);
  loadTaskList();
}

#endif
