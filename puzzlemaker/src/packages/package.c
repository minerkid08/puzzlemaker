#include "packages/package.h"
#include "assetManager.h"
#include "cjson.h"
#include "dynList.h"
#include "error.h"
#include "item/item.h"
#include "jsonUtils.h"
#include "packages/theme.h"

Package* currentPackage;

void loadPackageMetadata(Package* package)
{
	char buf[64];
	snprintf(buf, 64, "__%s__/package.json", package->name);
	int res;
	initErrorStackD(buf, res);
	if(res)
    startErrorLoop();

	int len;
	char* data = readPackageFile(package, "package.json", &len);
	data[len] = 0;

	cJSON* json = cJSON_Parse(data);
	const char* err = cJSON_GetErrorPtr();
	if (err)
		jsonParseError(data, err, buf);
	free(data);

	package->baseName = jsonGetStrD(json, "name");
	package->author = jsonGetStrD(json, "author");
	package->description = jsonGetStrD(json, "description");
	package->version = jsonGetStrD(json, "version");

  package->icon = -1;

	currentPackage = package;

  const char* filename = jsonGetStrC(json, "icon", 0);
  if(filename)
    package->icon = assetManagerLoadTexture(filename, 0);
  else
    package->icon = assetManagerLoadTexture("__base__/laser.png", 0);

	cJSON_Delete(json);
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
