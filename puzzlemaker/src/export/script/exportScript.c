#include "export/export.h"
#include "export/script/api.h"
#include "item/item.h"
#include "packages/package.h"
#include "utils.h"

#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <string.h>

static int handleError(lua_State* l)
{
	lua_pushvalue(l, 1);
	lua_pushstring(l, "\n");
	luaL_traceback(l, l, 0, 1);
	lua_concat(l, 3);
	return 1;
}

static Item* item;

static const char* getnextfilename(char** path, char* end)
{
	char* sep;
	char* name = *path;
	if (name == end)
		return 0; /* no more names */
	else if (*name == '\0')
	{				 /* from previous iteration? */
		*name = ';'; /* restore separator */
		name++;		 /* skip it */
	}
	sep = strchr(name, ';'); /* find next separator */
	if (sep == 0)			 /* separator not found? */
		sep = end;			 /* name goes until the end */
	*sep = '\0';			 /* finish file name */
	*path = sep;			 /* will start next search from here */
	return name;
}

static int searcher(lua_State* l)
{
	int offset = 0;
	const char* filename = luaL_checkstring(l, -1);
	const char* path = "assets/scripts/?.lua;assets/scripts/?/init.lua";

	filename = luaL_gsub(l, filename, ".", "/");
	Package* package = processPackageString(item->def->package, filename, &offset);

	luaL_Buffer b;
	luaL_buffinit(l, &b);
	luaL_addgsub(&b, path, "?", filename + offset);
	luaL_addchar(&b, 0);

	char* pathList = luaL_buffaddr(&b);
	char* endPathList = pathList + luaL_bufflen(&b) - 1;
	const char* file;

	while ((file = getnextfilename(&pathList, endPathList)) != 0)
	{
		if (packageContainsFile(package, file))
		{
			int fileLen;
			void* data = readPackageFile(package, file, &fileLen);
			char* buf = strdupf("@__%s__/%s", package->name, file);

			if (luaL_loadbuffer(l, data, fileLen, buf))
			{
				free(buf);
				free(data);
				return luaL_error(l, "error loading module '%s' from file '%s':\n\t%s", lua_tostring(l, 1), file,
								  lua_tostring(l, -1));
			}

			free(buf);
			free(data);
			lua_pushstring(l, file);
			return 2;
		}
	}

	pathList = luaL_buffaddr(&b);
	luaL_Buffer errBuf;
	luaL_buffinit(l, &errBuf);
	luaL_addstring(&errBuf, "no file '");
	luaL_addgsub(&errBuf, pathList, ";", "'\n\tno file '");
	luaL_addstring(&errBuf, "'");
	luaL_pushresult(&errBuf);
	return 1;
}

static int test(lua_State* l)
{
	luaL_error(l, "test");
	return 0;
}

int runExportScript(Item* item2)
{
	item = item2;

	lua_State* l = luaL_newstate();
	luaL_openlibs(l);

	lua_getglobal(l, "package");
	lua_getfield(l, -1, "searchers");
	lua_pushcfunction(l, searcher);
	lua_seti(l, -2, 2);
	lua_pushnil(l);
	lua_seti(l, -2, 3);
	lua_pushnil(l);
	lua_seti(l, -2, 4);
	lua_pop(l, 2);

	lua_pushnil(l);
	lua_setglobal(l, "io");

	lua_pushcfunction(l, test);
	lua_setglobal(l, "test");

	addBrushApi(l);
	addUtilsApi(l);
	addEntityApi(l, item);
	addItemApi(l, item);

	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "assets/scripts/%s", item->def->exportScript);
	int fileLen;
	int offset = 0;
	Package* package = processPackageString(item->def->package, buf, &offset);
	void* data = readPackageFile(package, buf, &fileLen);

	char* buf2 = strdupf("@__%s__/%s", package->baseName, buf + offset);
	writeMessagef("running script '%s' on item %s %d\n", buf2 + 1, item->def->name, item->index);

	if (luaL_loadbuffer(l, data, fileLen, buf2))
	{
		free(buf2);
		free(data);
		writeMessagef("%s\n", lua_tostring(l, -1));
		lua_close(l);
		return -1;
	}

	lua_pushcfunction(l, handleError);
	lua_pushvalue(l, -2);

	free(buf2);
	free(data);
	if (lua_pcall(l, 0, LUA_MULTRET, -2))
	{
		writeMessagef("%s\n", lua_tostring(l, -1));
		lua_close(l);
		return -1;
	}

	lua_close(l);
	return 0;
}
