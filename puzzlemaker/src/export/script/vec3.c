#include <cglm/cglm.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>

void getVec3(lua_State* l, int pos, vec3 out, const char* arg)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_error(l, "%s, expected vec3 [number, number, number]", arg);

	lua_geti(l, pos, 1);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_error(l, "%s x, expected vec3 [number, number, number]", arg);
	out[0] = lua_tonumber(l, -1);
	lua_geti(l, pos, 2);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_error(l, "%s y, expected vec3 [number, number, number]", arg);
	out[1] = lua_tonumber(l, -1);
	lua_geti(l, pos, 3);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_error(l, "%s z, expected vec3 [number, number, number]", arg);
	out[2] = lua_tonumber(l, -1);
	lua_pop(l, 3);
}
