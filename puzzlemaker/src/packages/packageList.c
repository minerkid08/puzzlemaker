#include "cjson.h"
#include "item/item.h"
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

#ifdef __linux__
__attribute__((constructor)) static void packageListInit()
#else
void packageListInit()
#endif
{
	packageList = dynList_new(0, sizeof(Package));
}

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)

__declspec(allocate(".CRT$XCU")) void (*packageListInitPtr)(void) = packageListInit;

#pragma comment(linker, "/include:packageListInitPtr")
#endif

void loadInstances();
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
				free(files[i]);
				files[i] = 0;
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
		if (name == 0)
			continue;
		Package* package = &packageList[dirCount + j++];
		char buf[128];
		snprintf(buf, 128, "packages/%s", name);
		package->zip = zip_open(buf, ZIP_RDONLY, 0);
		if (package->zip == 0)
			errorf("failed to open zip %s", name);
		int nameLen = strlen(name);
		for (int k = 0; k < nameLen; k++)
		{
			if (name[k] == '.')
			{
				((char*)name)[k] = 0;
				break;
			}
		}
		package->name = name;
		package->enabled = 1;
	}
	dynList_free(files);

	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		loadPackageMetadata(package);
	}

	metadataLoaded = 1;

	int res = 0;
	initErrorStackD("package.json", res);
	if (res)
		startErrorLoop();

	FILE* file = fopen("package.json", "rb");
	if (file)
	{
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

		for (int i = 0; i < packageCount; i++)
		{
			Package* package = &packageList[i];
			package->enabled = jsonGetBoolC(json, package->name, 1);
		}
		cJSON_Delete(json);
	}

	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		if (package->enabled)
			loadPackage(package);
		else
			zip_close(package->zip);
	}

	if (dynList_size(themeList) == 0)
		errorf("no themes found in any enabled package");

	dynList_trim((void**)&themeList);
	dynList_trim((void**)&packageList);

	setTheme(&themeList[0]);
	loadInstances();
	itemLoaderEnd();
}
