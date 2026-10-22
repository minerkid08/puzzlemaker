#include "cglm/vec2.h"
#include "export/script/api.h"
#include <cglm/cglm.h>
#include <lua/lauxlib.h>
#include <lua/lua.h>

// vec2.new(number x, number y): vec2
static int vec2New(lua_State* l);
// vec2.add(vec2 a, vec2 b): vec2
static int vec2Add(lua_State* l);
// vec2.sub(vec2 a, vec2 b): vec2
static int vec2Sub(lua_State* l);
// vec2.length(vec2 vec): number
static int vec2Length(lua_State* l);
// vec2.normalize(vec2 vec): vec2
static int vec2Normalize(lua_State* l);
// vec2.scale(vec2 vec, number scale): vec2
static int vec2Scale(lua_State* l);
// vec2.dot(vec2 a, vec2 b): vec2
static int vec2Dot(lua_State* l);
// vec2.cross(vec2 a, vec2 b): number
static int vec2Cross(lua_State* l);

void luaGetVec2(lua_State* l, int pos, vec2 out)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_typeerror(l, pos, "vec2");

	lua_geti(l, pos, 1);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_argerror(l, pos, "bad type for index 1, expected number");
	out[0] = lua_tonumber(l, -1);
	lua_geti(l, pos, 2);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_argerror(l, pos, "bad type for index 2, expected number");
	out[1] = lua_tonumber(l, -1);
	lua_pop(l, 2);
}

void luaPushVec2(lua_State* l, vec2 vec)
{
	lua_newtable(l);
	luaL_getmetatable(l, "vec2");
	lua_setmetatable(l, -2);
	lua_pushnumber(l, vec[0]);
	lua_seti(l, -2, 1);
	lua_pushnumber(l, vec[1]);
	lua_seti(l, -2, 2);
	lua_pushnumber(l, vec[2]);
	lua_seti(l, -2, 3);
}

void addVec2Api(lua_State* l)
{
	lua_newtable(l);

	luaL_newmetatable(l, "vec2");
	lua_newtable(l);
	lua_pushcfunction(l, vec2Add);
	lua_setfield(l, -2, "add");
	lua_pushcfunction(l, vec2Sub);
	lua_setfield(l, -2, "sub");
	lua_pushcfunction(l, vec2Length);
	lua_setfield(l, -2, "length");
	lua_pushcfunction(l, vec2Normalize);
	lua_setfield(l, -2, "normalize");
	lua_pushcfunction(l, vec2Scale);
	lua_setfield(l, -2, "scale");
	lua_pushcfunction(l, vec2Dot);
	lua_setfield(l, -2, "dot");
	lua_pushcfunction(l, vec2Cross);
	lua_setfield(l, -2, "cross");
	lua_setfield(l, -2, "__index");
	lua_pushstring(l, "vec2");
	lua_setfield(l, -2, "__name");

	lua_setmetatable(l, -2);
	lua_pushcfunction(l, vec2New);
	lua_setfield(l, -2, "new");

	lua_setglobal(l, "vec2");
}

static int vec2New(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TNUMBER)
		luaL_error(l, "bad arg 1, expected number");
	if (lua_type(l, 2) != LUA_TNUMBER)
		luaL_error(l, "bad arg 2, expected number");

	vec2 vec;
	vec[0] = lua_tonumber(l, 1);
	vec[1] = lua_tonumber(l, 2);

	luaPushVec2(l, vec);
	return 1;
}

static int vec2Add(lua_State* l)
{
	vec2 a;
	vec2 b;
	luaGetVec2(l, 1, a);
	luaGetVec2(l, 2, b);

	vec2 vec;
	vec[0] = a[0] + b[0];
	vec[1] = a[1] + b[1];

	luaPushVec2(l, vec);
	return 1;
}

static int vec2Sub(lua_State* l)
{
	vec2 a;
	vec2 b;
	luaGetVec2(l, 1, a);
	luaGetVec2(l, 2, b);

	vec2 vec;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];

	luaPushVec2(l, vec);
	return 1;
}

static int vec2Length(lua_State* l)
{
	vec2 vec;
	luaGetVec2(l, 1, vec);
	float len = sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
	lua_pushnumber(l, len);
	return 1;
}

static int vec2Normalize(lua_State* l)
{
	vec2 vec;
	luaGetVec2(l, 1, vec);
	glm_vec2_normalize(vec);
	luaPushVec2(l, vec);
	return 1;
}

static int vec2Scale(lua_State* l)
{
	vec2 vec;
	luaGetVec2(l, 1, vec);
	if (lua_type(l, 2) != LUA_TNUMBER)
		luaL_typeerror(l, 2, "number");

	float s = lua_tonumber(l, 2);
	vec[0] *= s;
	vec[1] *= s;

	luaPushVec2(l, vec);
	return 1;
}

static int vec2Dot(lua_State* l)
{
	vec2 a;
	vec2 b;
	luaGetVec2(l, 1, a);
	luaGetVec2(l, 2, b);

	float dot = glm_vec2_dot(a, b);
	lua_pushnumber(l, dot);
	return 1;
}

static int vec2Cross(lua_State* l)
{
	vec2 a;
	vec2 b;
	luaGetVec2(l, 1, a);
	luaGetVec2(l, 2, b);

	vec2 vec;
	float cross = glm_vec2_cross(a, b);

	lua_pushnumber(l, cross);
	return 1;
}
