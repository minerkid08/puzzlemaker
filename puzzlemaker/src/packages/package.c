#include "packages/package.h"
#include "assetManager.h"
#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "packages/theme.h"

Package* currentPackage;

static char* buf2;

void loadPackageMetadataInt()
{
	int len;
	char* data = readPackageFile(currentPackage, "package.json", &len);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	if (err)
		jsonParseError(data, err, buf2);
	free(data);

	currentPackage->baseName = jsonGetStrD(json, "name");
	currentPackage->author = jsonGetStrD(json, "author");
	currentPackage->description = jsonGetStrD(json, "description");
	currentPackage->version = jsonGetStrD(json, "version");

  currentPackage->icon = -1;

  const char* filename = jsonGetStrC(json, "icon", 0);
  if(filename)
    currentPackage->icon = assetManagerLoadTexture(filename);
  else
    currentPackage->icon = assetManagerLoadTexture("__base__/laser.png");

	cJSON_Delete(json);
}

void loadPackageMetadata(Package* package)
{
	char buf[64];
  buf2 = buf;
	snprintf(buf, 64, "__%s__/package.json", package->name);
  currentPackage = package;
	if(initErrorStack(buf, loadPackageMetadataInt))
    startErrorLoop();
}

void loadPackage(Package* package)
{
	currentPackage = package;
	printf("[package loader] loading package %s\n", package->name);

	const char** files = dynList_new(0, sizeof(const char*));

	listPackageFiles(package, "items", 0, &files, ".json");

	int len = dynList_size(files);
	for (int i = 0; i < len; i++)
	{
		loadItemDefinitionFile(package, files[i]);
		free((void*)files[i]);
	}

	dynList_resize((void**)&files, 0);

	listPackageFiles(package, "themes", 0, &files, ".json");

	len = dynList_size(files);
	for (int i = 0; i < len; i++)
	{
		loadTheme(files[i]);
		free((void*)files[i]);
	}

	dynList_free(files);
}
