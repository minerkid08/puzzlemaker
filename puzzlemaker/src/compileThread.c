#include "compileThread.h"
#include "dynList.h"
#include "jsonUtils.h"
#include <cjson.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define pipeRead pipefd[0]
#define pipeWrite pipefd[1]

static int pipefd[2];
static pthread_t thread;

static char name[64];

static int currentStep = 0;

static int stepCount = 0;
static CompileStep* compileSteps;

static const char* p2ce;
static const char* bin;

static char curPath[512];
static char buf[512];

void processString(const char* str)
{
	int len = strlen(str);
	char* ptr = buf;

	for (int i = 0; i < len; i++)
	{
		if (str[i] != '%')
		{
			*ptr = str[i];
			ptr++;
			continue;
		}

		if (str[i + 1] == ' ')
		{
			*ptr = '%';
			ptr++;
			*ptr = ' ';
			ptr++;
			i++;
		}

		if (str[i + 1] == 'f')
		{
			int l = snprintf(ptr, buf + 512 - ptr, "%s", name);
			ptr += l;
		}

		if (str[i + 1] == 'b')
		{
			int l = snprintf(ptr, buf + 512 - ptr, "%s", bin);
			ptr += l;
		}

		if (str[i + 1] == 'p')
		{
			int l = snprintf(ptr, buf + 512 - ptr, "%s", p2ce);
			ptr += l;
		}

		if (str[i + 1] == 'c')
		{
			int l = snprintf(ptr, buf + 512 - ptr, "%s", curPath);
			ptr += l;
		}
		i++;
	}
	*ptr = 0;
}

int runCmd(char* cmd)
{
	processString(cmd);

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

		char b2[512];
		j = 0;
		for (int i = 0; i < strlen(p2ce); i++)
		{
			if (p2ce[i] == '%')
			{
				continue;
			}
			b2[j] = p2ce[i];
			j++;
		}
		b2[j] = 0;

		if (chdir(b2))
			perror("chdir");

		execvp(data[0], data);
		perror("execlp");
		return 1;
	}
	else
	{
		// Parent process

		wait(0);
	}
	return 0;
}

void* compileThread(void* e)
{
	while (1)
	{
		char c;
		read(pipeRead, &c, 1);
		printf("compiling");
		fflush(stdout);
		currentStep = 0;
		for (int i = 0; i < stepCount; i++)
		{
			CompileStep* step = &compileSteps[i];
			runCmd((char*)step->cmd);
			if (currentStep == -1)
				break;
			currentStep++;
		}
	}
	return 0;
}

void cancelCompile()
{
	currentStep = -1;
}

void startCompile(const char* filename)
{
	printf("compileStarted");
	fflush(stdout);
	getcwd(curPath, 512);
	strncpy(name, filename, sizeof(name));
	char c = 1;
	write(pipeWrite, &c, 1);
}

int getCompileStep()
{
	return currentStep;
}

int getCompileStepCount()
{
	return stepCount;
}

CompileStep* getCompileSteps()
{
	return compileSteps;
}

void startCompileThread()
{
	pipe(pipefd);
	pthread_create(&thread, 0, compileThread, 0);

	FILE* file = fopen("compileSteps.json", "rb");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	free(data);
	cJSON* steps = cJSON_GetObjectItem(json, "steps");

	stepCount = cJSON_GetArraySize(steps);
	compileSteps = malloc(sizeof(CompileStep) * (stepCount + 1));
	for (int i = 0; i < stepCount; i++)
	{
		cJSON* item = cJSON_GetArrayItem(steps, i);
		CompileStep* step = &compileSteps[i];
		step->name = jsonGetStr(item, "name");
		step->cmd = jsonGetStr(item, "cmd");
	}
	p2ce = jsonGetStr(json, "p2ce");
	bin = jsonGetStr(json, "bin");

	CompileStep* step = &compileSteps[stepCount];
	step->name = "done";

	cJSON_free(json);
}
