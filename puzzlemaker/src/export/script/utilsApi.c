#include "renderer/texture.h"
#include "utils.h"
#include <lua/lua.h>

void addUtilsApi(lua_State* l)
{
	lua_newtable(l);
	lua_pushnumber(l, DIR_POS_X);
	lua_setfield(l, -2, "POS_X");
	lua_pushnumber(l, DIR_NEG_X);
	lua_setfield(l, -2, "NEG_X");
	lua_pushnumber(l, DIR_POS_Y);
	lua_setfield(l, -2, "POS_Y");
	lua_pushnumber(l, DIR_NEG_Y);
	lua_setfield(l, -2, "NEG_Y");
	lua_pushnumber(l, DIR_POS_Z);
	lua_setfield(l, -2, "POS_Z");
	lua_pushnumber(l, DIR_NEG_Z);
	lua_setfield(l, -2, "NEG_Z");
	lua_setglobal(l, "Direction");

	lua_newtable(l);
	lua_pushnumber(l, TEX_ALIGNMENT_NONE);
	lua_setfield(l, -2, "None");
	lua_pushnumber(l, TEX_ALIGNMENT_BOTTOM_LEFT);
	lua_setfield(l, -2, "BottomLeft");
	lua_pushnumber(l, TEX_ALIGNMENT_FIT);
	lua_setfield(l, -2, "Fit");
	lua_setglobal(l, "TextureAlignment");

}
