#pragma once
#include <zip.h>

typedef struct
{
  const char* name;
  const char* baseName;
  const char* author;
  const char* description;
  const char* version;
  unsigned int icon;
  char enabled;

	int itemGroupStart;
	int itemGroupCount;

	const char** requiredAddons;

  zip_t* zip;
} Package;

extern Package* currentPackage;

void loadPackageList();
void loadPackageMetadata(Package* package);
void loadPackage(Package* package);

Package* processPackageString(Package* package, const char* filename, int* offset);
char packageContainsFile(Package* package, const char* filename);
void* readPackageFile(Package* package, const char* filename, int* len);
int listPackageFiles(Package* package, const char* path, const char*** dirs, const char*** files, const char* extensionFilter);
