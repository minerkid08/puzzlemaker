#include "export/script/api.h"
#include "item/item.h"

#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>

int runExportScript(Item* item)
{
	lua_State* l = luaL_newstate();
	luaL_openlibs(l);

	luaL_dostring(l, "package.path = 'assets/scripts/?.lua;assets/scripts/?/init.lua';");

	lua_pushnil(l);
	lua_setglobal(l, "io");

	addBrushApi(l);
	addUtilsApi(l);
	addEntityApi(l, item);
	addItemApi(l, item);

	char buf[256];
	snprintf(buf, sizeof(buf), "assets/scripts/%s", item->def->exportScript);

	if (luaL_loadfile(l, buf))
	{
		printf("export script '%s' failed to compile for item %s %d\n", item->def->exportScript, item->def->name,
			   item->index);
		printf("%s\n", lua_tostring(l, -1));
		lua_close(l);
		return -1;
	}

	if (lua_pcall(l, 0, 0, 0))
	{
		printf("export script '%s' runtime error for item %s %d\n", item->def->exportScript, item->def->name,
			   item->index);
		printf("%s\n", lua_tostring(l, -1));
		lua_close(l);
		return -1;
	}

	lua_close(l);
	return 0;
}
