#include "compile/compileThread.h"
#include "dynList.h"
#include "packages/package.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <zip.h>

#ifdef __linux__
#include <sys/stat.h>
#else
#include <windows.h>
#endif

int off = 10;

extern CompileStatus compileStatus;
extern Package* packageList;

unsigned long long getWriteTime(const char* path)
{
#ifdef __linux__
	struct stat s;
	if (stat(path, &s))
		return 0;

	return (unsigned long long)s.st_mtim.tv_sec;
#else
	HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	FILETIME ftCreate, ftAccess, ftWrite;
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
	{
		CloseHandle(hFile);
		return 0;
	}

	CloseHandle(hFile);

	ULARGE_INTEGER u;
	u.u.LowPart = ftWrite.dwLowDateTime;
	u.u.HighPart = ftWrite.dwHighDateTime;

	return u.QuadPart;
#endif
}

void copyDir(Package* package, const char* dirname, const char* destDir)
{
	char** dirs = dynList_new(0, sizeof(char*));
	char** files = dynList_new(0, sizeof(char*));
	listPackageFiles(package, dirname, (const char***)&dirs, (const char***)&files, ".vmf");

	int dirCount = dynList_size(dirs);
	int fileCount = dynList_size(files);

	for (int i = 0; i < fileCount; i++)
	{
		char* filename = files[i];

		char* src = strdupf("%s/%s", dirname, filename);
		char* dest = strdupf("%s/%s%s", destDir, dirname + off, filename);

		if (package->zip)
		{
			zip_stat_t stat;
			zip_stat(package->zip, src, 0, &stat);

			unsigned long t = getWriteTime(dest);
			if (stat.mtime > t)
			{
				printf("copying '%s' to '%s'\n", src, dest);
				zip_file_t* file = zip_fopen(package->zip, src, 0);

				int len = stat.size;

				FILE* destFile = fopen(dest, "wb");

				int read = 0;
				char buf[512];
				while (read < len)
				{
					int r = zip_fread(file, buf, 512);
					fwrite(buf, 1, r, destFile);
					read += r;
				}
				fclose(destFile);
				zip_fclose(file);
			}
		}
		else
		{
			char path[128];
			snprintf(path, 128, "packages/%s/%s", package->name, src);

			unsigned long srcTime = getWriteTime(path);
			unsigned long dstTime = getWriteTime(dest);

			if (srcTime > dstTime)
			{
				printf("copying '%s' to '%s'\n", path, dest);
				FILE* file = fopen(path, "rb");

				fseek(file, 0, SEEK_END);
				int len = ftell(file);
				fseek(file, 0, SEEK_SET);

				FILE* destFile = fopen(dest, "wb");

				int read = 0;
				char buf[512];
				while (read < len)
				{
					int r = fread(buf, 1, 512, file);
					fwrite(buf, 1, r, destFile);
					read += r;
				}
				fclose(destFile);
				fclose(file);
			}
		}

		free(src);
		free(dest);
		free(filename);
	}
	dynList_free(files);
	for (int i = 0; i < dirCount; i++)
	{
		char* dir = dirs[i];
		char* src = strdupf("%s/%s", dirname, dir);
		char* dest;
		dest = strdupf("%s/%s/%s", destDir, dirname + off, dir);
		makeDir(dest);
		free(dest);
		copyDir(package, src, destDir);
		free(src);
		free(dir);
	}
	dynList_free(dirs);
}

void loadInstances()
{
	printf("[instance loader] loading instances\n");
	char* p2ce = strdup(compileStatus.p2ce);
	int j = 0;
	for (int k = 0; k < strlen(p2ce); k++)
	{
		if (p2ce[k] == '%')
			continue;
		p2ce[j++] = p2ce[k];
	}
	p2ce[j] = 0;
	char* path = strdupf("%s/%s/pmInstances/", p2ce, compileStatus.maps);
	makeDir(path);
	printf("[instance loader] instance dest dir '%s'\n", path);
	free(path);
	path = strdupf("%s/%s/puzzlemakerMaps", p2ce, compileStatus.maps);
	makeDir(path);
	free(path);

	int packageCount = dynList_size(packageList);
	for (int i = 0; i < packageCount; i++)
	{
		Package* package = &packageList[i];
		if (package->enabled == 0)
			continue;
		char* path = strdupf("%s/%s/pmInstances/%s", p2ce, compileStatus.maps, package->name);
		makeDir(path);
		copyDir(package, "instances", path);
		free(path);
	}
	free(p2ce);
}
