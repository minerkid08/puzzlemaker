#include "dynList.h"
#include "packages/package.h"
#include "utils.h"
#include <string.h>
#include <zip.h>

extern Package* packageList;

static int pstrcmp(const void* a, const void* b)
{
	return strcmp(*(const char**)a, *(const char**)b);
}

Package* processPackageString(Package* p, const char* filename, int* offset)
{
	Package* package = p;
	if (startsWith(filename, "__"))
	{
		char packageName[64];
		int len = strlen(filename);
		int j = 0;
		for (int i = 2; i < len; i++)
		{
			if (filename[i] == '_' && filename[i + 1] == '_')
			{
				*offset = i + 3;
				break;
			}
			packageName[j++] = filename[i];
		}
		packageName[j] = 0;
		len = dynList_size(packageList);
		package = 0;
		for (int i = 0; i < len; i++)
		{
			Package* p = &packageList[i];
			if (!p->enabled)
				continue;
			if (strcmp(p->name, packageName) == 0)
			{
				package = p;
				break;
			}
		}
		if (package == 0)
			errorf("error loading %s\nno enabled package with the name %s", filename, packageName);
	}
	return package;
}

char packageContainsFile(Package* package, const char* filename)
{
	if (package->zip == 0)
	{
		char path[128];
		snprintf(path, 128, "packages/%s/%s", package->name, filename);

		FILE* file = fopen(path, "rb");
		if (file == 0)
			return 0;
		fclose(file);
		return 1;
	}
	else
	{
		zip_file_t* file = zip_fopen(package->zip, filename, 0);

		if (file == 0)
			return 0;
		zip_fclose(file);
		return 1;
	}
}

void* readPackageFile(Package* package, const char* filename, int* plen)
{
	if (package->zip == 0)
	{
		char path[128];
		snprintf(path, 128, "packages/%s/%s", package->name, filename);

		FILE* file = fopen(path, "rb");
		if (file == 0)
			errorf("failed to open __%s__/%s", package->name, filename);

		fseek(file, 0, SEEK_END);
		int len = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* data = malloc(len + 1);

		fread(data, 1, len, file);
		*plen = len;
		return data;
	}
	else
	{
		zip_file_t* file = zip_fopen(package->zip, filename, 0);

		if (file == 0)
			errorf("failed to open __%s__/%s", package->name, filename);

		zip_stat_t stat;
		zip_stat(package->zip, filename, 0, &stat);

		int len = stat.size;

		char* data = malloc(len + 1);

		zip_fread(file, data, len);
		*plen = len;
		return data;
	}
}

int listPackageFiles(Package* package, const char* path, const char*** dirs, const char*** files,
					 const char* extensionFilter)
{
	if (package->zip == 0)
	{
		char buf[128];
		snprintf(buf, 128, "packages/%s/%s", package->name, path);
		return listFiles(buf, dirs, files, extensionFilter);
	}

	int fileCount = zip_get_num_entries(package->zip, 0);

	for (int fileInd = 0; fileInd < fileCount; fileInd++)
	{
		const char* name = zip_get_name(package->zip, fileInd, 0);
		if (startsWith(name, path))
		{
			int nameLen = strlen(name);
			if (name[nameLen - 1] == '/')
			{
				if (dirs)
				{
					int len = strlen(name);
					int j = 0;
					for (int i = 0; i < len - 1; i++)
					{
						if (name[i] == '/')
							j = i;
					}
					if (j == 0)
						continue;
					int l = dynList_size(*dirs);
					dynList_resize((void**)dirs, l + 1);
					(*dirs)[l] = strdup(name + j + 1);
				}
			}
			else
			{
				if (extensionFilter)
				{
					int len = strlen(name);
					int extensionLen = strlen(extensionFilter);
					if (strcmp(name + len - extensionLen, extensionFilter) == 0)
					{
						int l = dynList_size(*files);
						dynList_resize((void**)files, l + 1);
						int j = 0;
						for (int i = 0; i < len; i++)
						{
							if (name[i] == '/')
								j = i;
						}
						(*files)[l] = strdup(name + j + 1);
					}
				}
				else
				{
					int len = strlen(name);
					int l = dynList_size(*files);
					dynList_resize((void**)files, l + 1);
					int j = 0;
					for (int i = 0; i < len; i++)
					{
						if (name[i] == '/')
							j = i;
					}
					(*files)[l] = strdup(name + j + 1);
				}
			}
		}
	}
	if (files)
	{
		int fileCount = dynList_size(*files);
		qsort(*files, fileCount, sizeof(const char*), pstrcmp);
	}
	else
	{
		int dirCount = dynList_size(*dirs);
		qsort(*dirs, dirCount, sizeof(const char*), pstrcmp);
	}
	return 0;
}
