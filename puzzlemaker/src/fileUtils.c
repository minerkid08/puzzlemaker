#include "dynList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pstrcmp(const void* a, const void* b)
{
	return strcmp(*(const char**)a, *(const char**)b);
}

#ifdef linux
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>

void listFiles(const char* path, const char*** dirs, const char*** files, const char* extensionFilter)
{
	struct dirent* en;

	DIR* dir = opendir(path);
	if (dir)
	{
		while ((en = readdir(dir)) != 0)
		{
			if (strcmp(en->d_name, ".") == 0)
				continue;
			if (strcmp(en->d_name, "..") == 0)
				continue;
			if (en->d_type == DT_DIR && dirs)
			{
				int len = dynList_size(*dirs);
				dynList_resize((void**)dirs, len + 1);
				(*dirs)[len] = strdup(en->d_name);
			}
			else if (files)
			{
				if (extensionFilter)
				{
					int len = strlen(en->d_name);
					int extensionLen = strlen(extensionFilter);
					if (strcmp(en->d_name + len - extensionLen, extensionFilter) == 0)
					{
						int l = dynList_size(*files);
						dynList_resize((void**)files, l + 1);
						(*files)[l] = strdup(en->d_name);
					}
				}
				else
				{
					int l = dynList_size(*files);
					dynList_resize((void**)files, l + 1);
					(*files)[l] = strdup(en->d_name);
				}
			}
		}
		closedir(dir);
	}
	else
		printf("bad directory '%s'\n", path);
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
}

void makeDir(const char* path)
{
	mkdir(path, 0777);
}

#else
#include <windows.h>

void listFiles(const char* dirPath, const char*** dirs, const char*** files, const char* extensionFilter)
{
	char searchPath[MAX_PATH];
	snprintf(searchPath, sizeof(searchPath), "%s\\*", dirPath);

	WIN32_FIND_DATA en;
	HANDLE hFind = FindFirstFile(searchPath, &en);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open directory: %s\n", dirPath);
		return;
	}

	do
	{
		if (strcmp(en.cFileName, ".") != 0 && strcmp(en.cFileName, "..") != 0)
		{

			if (en.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (dirs)
				{
					int len = dynList_size(*dirs);
					dynList_resize((void**)dirs, len + 1);
					(*dirs)[len] = strdup(en.cFileName);
				}
			}
			else
			{
				if (files)
				{
					if (extensionFilter)
					{
						int len = strlen(en.cFileName);
						int extensionLen = strlen(extensionFilter);
						if (strcmp(en.cFileName + len - extensionLen, extensionFilter) == 0)
						{
							int l = dynList_size(*files);
							dynList_resize((void**)files, l + 1);
							(*files)[l] = strdup(en.cFileName);
						}
					}
					else
					{
						int l = dynList_size(*files);
						dynList_resize((void**)files, l + 1);
						(*files)[l] = strdup(en.cFileName);
					}
				}
			}
		}
	} while (FindNextFile(hFind, &en));

	FindClose(hFind);
	qsort(*files, dynList_size(*files), sizeof(const char*), pstrcmp);
	qsort(*dirs, dynList_size(*dirs), sizeof(const char*), pstrcmp);
}

void makeDir(const char* path)
{
	if (!CreateDirectory(path, NULL))
	{
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
			printf("Failed to create directory. Error code: %lu\n", error);
	}
}
#endif
