#include "cglm/vec3.h"
#include "export/script/api.h"
#include <cglm/cglm.h>
#include <lua/lauxlib.h>
#include <lua/lua.h>

// vec3.new(number x, number y, number z): vec3
static int vec3New(lua_State* l);
// vec3.add(vec3 a, vec3 b): vec3
static int vec3Add(lua_State* l);
// vec3.sub(vec3 a, vec3 b): vec3
static int vec3Sub(lua_State* l);
// vec3.length(vec3 vec): number
static int vec3Length(lua_State* l);
// vec3.normalize(vec3 vec): vec3
static int vec3Normalize(lua_State* l);
// vec3.scale(vec3 vec, number scale): vec3
static int vec3Scale(lua_State* l);
// vec3.dot(vec3 a, vec3 b): vec3
static int vec3Dot(lua_State* l);
// vec3.cross(vec3 a, vec3 b): vec3
static int vec3Cross(lua_State* l);

void luaGetVec3(lua_State* l, int pos, vec3 out)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_typeerror(l, pos, "vec3");

	lua_geti(l, pos, 1);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_argerror(l, pos, "bad type for index 1, expected number");
	out[0] = lua_tonumber(l, -1);
	lua_geti(l, pos, 2);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_argerror(l, pos, "bad type for index 2, expected number");
	out[1] = lua_tonumber(l, -1);
	lua_geti(l, pos, 3);
	if (lua_type(l, -1) != LUA_TNUMBER)
		luaL_argerror(l, pos, "bad type for index 3, expected number");
	out[2] = lua_tonumber(l, -1);
	lua_pop(l, 3);
}

void luaPushVec3(lua_State* l, vec3 vec)
{
	lua_newtable(l);
	luaL_getmetatable(l, "vec3");
	lua_setmetatable(l, -2);
	lua_pushnumber(l, vec[0]);
	lua_seti(l, -2, 1);
	lua_pushnumber(l, vec[1]);
	lua_seti(l, -2, 2);
	lua_pushnumber(l, vec[2]);
	lua_seti(l, -2, 3);
}

void addVec3Api(lua_State* l)
{
	lua_newtable(l);

	luaL_newmetatable(l, "vec3");
	lua_newtable(l);
	lua_pushcfunction(l, vec3Add);
	lua_setfield(l, -2, "add");
	lua_pushcfunction(l, vec3Sub);
	lua_setfield(l, -2, "sub");
	lua_pushcfunction(l, vec3Length);
	lua_setfield(l, -2, "length");
	lua_pushcfunction(l, vec3Normalize);
	lua_setfield(l, -2, "normalize");
	lua_pushcfunction(l, vec3Scale);
	lua_setfield(l, -2, "scale");
	lua_pushcfunction(l, vec3Dot);
	lua_setfield(l, -2, "dot");
	lua_pushcfunction(l, vec3Cross);
	lua_setfield(l, -2, "cross");
	lua_setfield(l, -2, "__index");
	lua_pushstring(l, "vec2");
	lua_setfield(l, -2, "__name");

	lua_setmetatable(l, -2);
	lua_pushcfunction(l, vec3New);
	lua_setfield(l, -2, "new");
	vec3 fwd = {0, 0, 1};
	luaPushVec3(l, fwd);
	lua_setfield(l, -2, "forward");
	vec3 right = {-1, 0, 0};
	luaPushVec3(l, right);
	lua_setfield(l, -2, "right");
	vec3 up = {0, 1, 0};
	luaPushVec3(l, up);
	lua_setfield(l, -2, "up");

	lua_setglobal(l, "vec3");
}

static int vec3New(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TNUMBER)
		luaL_typeerror(l, 1, "number");
	if (lua_type(l, 2) != LUA_TNUMBER)
		luaL_typeerror(l, 2, "number");
	if (lua_type(l, 3) != LUA_TNUMBER)
		luaL_typeerror(l, 3, "number");

	vec3 vec;
	vec[0] = lua_tonumber(l, 1);
	vec[1] = lua_tonumber(l, 2);
	vec[2] = lua_tonumber(l, 3);

	luaPushVec3(l, vec);
	return 1;
}

static int vec3Add(lua_State* l)
{
	vec3 a;
	vec3 b;
	luaGetVec3(l, 1, a);
	luaGetVec3(l, 2, b);

	vec3 vec;
	vec[0] = a[0] + b[0];
	vec[1] = a[1] + b[1];
	vec[2] = a[2] + b[2];

	luaPushVec3(l, vec);
	return 1;
}

static int vec3Sub(lua_State* l)
{
	vec3 a;
	vec3 b;
	luaGetVec3(l, 1, a);
	luaGetVec3(l, 2, b);

	vec3 vec;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];
	vec[2] = a[2] - b[2];

	luaPushVec3(l, vec);
	return 1;
}

static int vec3Length(lua_State* l)
{
	vec3 vec;
	luaGetVec3(l, 1, vec);
	float len = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	lua_pushnumber(l, len);
	return 1;
}

static int vec3Normalize(lua_State* l)
{
	vec3 vec;
	luaGetVec3(l, 1, vec);
	glm_normalize(vec);
	luaPushVec3(l, vec);
	return 1;
}

static int vec3Scale(lua_State* l)
{
	vec3 vec;
	luaGetVec3(l, 1, vec);
	if (lua_type(l, 2) != LUA_TNUMBER)
		luaL_typeerror(l, 2, "number");

	float s = lua_tonumber(l, 2);
	vec[0] *= s;
	vec[1] *= s;
	vec[2] *= s;

	luaPushVec3(l, vec);
	return 1;
}

static int vec3Dot(lua_State* l)
{
	vec3 a;
	vec3 b;
	luaGetVec3(l, 1, a);
	luaGetVec3(l, 2, b);

	float dot = glm_dot(a, b);
	lua_pushnumber(l, dot);
	return 1;
}

static int vec3Cross(lua_State* l)
{
	vec3 a;
	vec3 b;
	luaGetVec3(l, 1, a);
	luaGetVec3(l, 2, b);

	vec3 vec;
	glm_vec3_cross(a, b, vec);

	luaPushVec3(l, vec);
	return 1;
}
