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

void luaGetVec2(lua_State* l, int pos, vec2 out, const char* arg)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_error(l, "%s, expected vec2 [number, number, number]", arg);

	lua_geti(l, pos, 1);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_error(l, "%s x, expected vec2 [number, number, number]", arg);
	out[0] = lua_tonumber(l, -1);
	lua_geti(l, pos, 2);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_error(l, "%s y, expected vec2 [number, number, number]", arg);
	out[1] = lua_tonumber(l, -1);
	lua_geti(l, pos, 3);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_error(l, "%s z, expected vec2 [number, number, number]", arg);
	out[2] = lua_tonumber(l, -1);
	lua_pop(l, 3);
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
	luaGetVec2(l, 1, a, "bad arg 1");
	luaGetVec2(l, 2, b, "bad arg 2");

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
	luaGetVec2(l, 1, a, "bad arg 1");
	luaGetVec2(l, 2, b, "bad arg 2");

	vec2 vec;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];

	luaPushVec2(l, vec);
	return 1;
}

static int vec2Length(lua_State* l)
{
	vec2 vec;
	luaGetVec2(l, 1, vec, "bad arg 1");
	float len = sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
	lua_pushnumber(l, len);
	return 1;
}

static int vec2Normalize(lua_State* l)
{
	vec2 vec;
	luaGetVec2(l, 1, vec, "bad arg 1");
	glm_vec2_normalize(vec);
	luaPushVec2(l, vec);
	return 1;
}

static int vec2Scale(lua_State* l)
{
	vec2 vec;
	luaGetVec2(l, 1, vec, "bad arg 1");
	if (lua_type(l, 2) != LUA_TNUMBER)
		luaL_error(l, "bad arg 2, expected number");

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
	luaGetVec2(l, 1, a, "bad arg 1");
	luaGetVec2(l, 2, b, "bad arg 2");

	float dot = glm_vec2_dot(a, b);
	lua_pushnumber(l, dot);
	return 1;
}

static int vec2Cross(lua_State* l)
{
	vec2 a;
	vec2 b;
	luaGetVec2(l, 1, a, "bad arg 1");
	luaGetVec2(l, 2, b, "bad arg 2");

	vec2 vec;
	float cross = glm_vec2_cross(a, b);

	lua_pushnumber(l, cross);
	return 1;
}
