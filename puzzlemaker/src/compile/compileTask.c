#include "cjson.h"
#include "compileThread.h"
#include "jsonUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern CompileStatus compileStatus;

void loadTaskList()
{
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

	compileStatus.stepCount = cJSON_GetArraySize(steps);
	compileStatus.compileSteps = malloc(sizeof(CompileStep) * (compileStatus.stepCount + 2));
	for (int i = 0; i < compileStatus.stepCount; i++)
	{
		cJSON* item = cJSON_GetArrayItem(steps, i);
		CompileStep* step = &compileStatus.compileSteps[i + 1];
		step->name = jsonGetStr(item, "name");
		step->cmd = jsonGetStr(item, "cmd");
	}
	compileStatus.p2ce = jsonGetStr(json, "p2ce");
	compileStatus.bin = jsonGetStr(json, "bin");

	compileStatus.stepCount++;
	CompileStep* step = &compileStatus.compileSteps[compileStatus.stepCount];
	step->name = "done";

	step = &compileStatus.compileSteps[0];
	step->name = "generate vmf";

	cJSON_free(json);

	int j = 0;
	for (int i = 0; i < strlen(compileStatus.p2ce); i++)
	{
		if (compileStatus.p2ce[i] == '%')
			continue;
		compileStatus.workingDir[j] = compileStatus.p2ce[i];
		j++;
	}
	compileStatus.workingDir[j] = 0;
}
