#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "jsonUtils.h"
#include "packages/package.h"
#include "packages/theme.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <zip.h>

Package* packageList;
char metadataLoaded = 0;

static __attribute__((constructor)) void init()
{
	packageList = dynList_new(0, sizeof(Package));
}

static void loadPackageFile()
{
	FILE* file = fopen("package.json", "rb");
	if (file == 0)
		return;
	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	free(data);
	if (err)
		jsonParseError(data, err, "package.json");

	int packageCount = dynList_size(packageList);
	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		package->enabled = jsonGetBoolC(json, package->name, 1);
	}
	cJSON_Delete(json);
}

void loadPackageList()
{
	char** dirs = dynList_new(0, sizeof(char*));
	char** files = dynList_new(0, sizeof(char*));
	listFiles("packages", (const char***)&dirs, (const char***)&files, ".zip");

	int dirCount = dynList_size(dirs);
	int fileCount = dynList_size(files);

	int packageCount = dirCount + fileCount;

	for (int i = 0; i < fileCount; i++)
	{
		char buf[64];
		strncpy(buf, files[i], 64);
		int l = strlen(buf);
		l -= 4;
		buf[l] = 0;

		for (int j = 0; j < dirCount; j++)
		{
			if (strcmp(buf, dirs[j]) == 0)
			{
				files[i][0] = 0;
				packageCount--;
				break;
			}
		}
	}

	dynList_resize((void**)&packageList, packageCount);

	for (int i = 0; i < dirCount; i++)
	{
		const char* name = dirs[i];
		Package* package = &packageList[i];
		package->name = name;
		package->enabled = 1;
		package->zip = 0;
	}
	dynList_free(dirs);

	int j = 0;
	for (int i = 0; i < fileCount; i++)
	{
		const char* name = files[i];
		if (name[0] == 0)
			continue;
		Package* package = &packageList[dirCount + j++];
		package->name = name;
		package->enabled = 1;
		char buf[128];
		snprintf(buf, 128, "packages/%s", name);
		package->zip = zip_open(buf, ZIP_RDONLY, 0);
		if (package->zip == 0)
			errorf("failed to open zip %s\n", name);
	}
	dynList_free(files);

	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		loadPackageMetadata(package);
	}

	metadataLoaded = 1;

	if (initErrorStack("package.json", loadPackageFile))
		startErrorLoop();

	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		if (package->enabled)
			loadPackage(package);
		else
			zip_close(package->zip);
	}

	if (dynList_size(themeList) == 0)
		errorf("no themes found in any enabled package\n");

	setTheme(&themeList[0]);
}
