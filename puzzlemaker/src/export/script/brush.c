#include "export/brush.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include <string.h>
#include "api.h"
#include "utils.h"

static int brushNew(lua_State* l);
static int brushSetTexture(lua_State* l);
static int brushTransform(lua_State* l);

Brush* luaGetBrush(lua_State* l, int pos, const char* arg)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_error(l, "%s, expected Brush", arg);

	lua_getfield(l, pos, "id");
	if (lua_type(l, -1) != LUA_TLIGHTUSERDATA)
		luaL_error(l, "%s, invalid Brush", arg);

	long long id = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);
	return &getBrushArray()[id];
}

void addBrushApi(lua_State* l)
{
	lua_newtable(l);
	lua_pushcfunction(l, brushNew);
	lua_setfield(l, -2, "new");
	lua_pushcfunction(l, brushSetTexture);
	lua_setfield(l, -2, "setTexture");
	lua_pushcfunction(l, brushTransform);
	lua_setfield(l, -2, "transform");
	lua_setglobal(l, "Brush");
}

static int brushNew(lua_State* l)
{
	vec3 start;
	vec3 end;

	luaGetVec3(l, 1, start, "bad arg 1");
	luaGetVec3(l, 2, end, "bad arg 2");

	Brush* b = exportCreateBrush(start, end);
	b->script = 1;

	long long id = b->id;
	lua_newtable(l);
	lua_pushlightuserdata(l, (void*)id);
	lua_setfield(l, -2, "id");
	lua_pushcfunction(l, brushSetTexture);
	lua_setfield(l, -2, "setTexture");
	lua_pushcfunction(l, brushTransform);
	lua_setfield(l, -2, "transform");
	return 1;
}

static int brushSetTexture(lua_State* l)
{
	if(lua_gettop(l) == 3)
		lua_pushnil(l);

	Brush* brush = luaGetBrush(l, 1, "bad arg 1");

	if (lua_type(l, 2) != LUA_TNUMBER)
		luaL_error(l, "bad arg 2, expected number");
	if (lua_type(l, 3) != LUA_TSTRING)
		luaL_error(l, "bad arg 3, expected string");

	char type = lua_type(l, 4);
	if (type != LUA_TNIL && type != LUA_TTABLE)
		luaL_error(l, "bad arg 4, expected table?");

	char dir = lua_tonumber(l, 2);
	if (dir < 0 || dir > 5)
		luaL_error(l, "bad arg 2, dir value is out of range");

	const char* texName = lua_tostring(l, 3);

	Side* side = &brush->sides[dir];

	if (side->material)
		free((char*)side->material);
	side->material = strdup(texName);

	if (type == LUA_TTABLE)
	{
		lua_getfield(l, 4, "texSize");
		type = lua_type(l, -1);
		if (type != LUA_TNIL)
		{
			if (type != LUA_TNUMBER)
				luaL_error(l, "bad field 'texSize' in arg 4, expected number?");
			side->texHeight = lua_tonumber(l, -1);
			side->texWidth = side->texHeight;
		}
		lua_pop(l, 1);

		lua_getfield(l, 4, "lightmapScale");
		type = lua_type(l, -1);
		if (type != LUA_TNIL)
		{
			if (type != LUA_TNUMBER)
				luaL_error(l, "bad field 'lightmapScale' in arg 4, expected number?");
			side->lightmapscale = lua_tonumber(l, -1);
		}
		lua_pop(l, 1);

		lua_getfield(l, 4, "fit");
		type = lua_type(l, -1);
		if (type != LUA_TNIL)
		{
			if (type != LUA_TBOOLEAN)
				luaL_error(l, "bad field 'fit' in arg 4, expected bool?");
			side->fit = lua_toboolean(l, -1);
		}
		lua_pop(l, 1);
	}

	return 0;
}

static int brushTransform(lua_State* l)
{
	Brush* brush = luaGetBrush(l, 1, "bad arg 1");

	vec3 pos;
	vec3 rot;

	luaGetVec3(l, 2, pos, "bad arg 2");
	luaGetVec3(l, 3, rot, "bad arg 2");

	mat4 transform;
	glm_mat4_identity(transform);
	glm_translate(transform, pos);

	vec3 dir;
	vec4 quat;
	dir[0] = glm_rad(rot[0]);
	dir[1] = glm_rad(rot[1]);
	dir[2] = glm_rad(rot[2]);
	glm_euler_yxz_quat(dir, quat);
	mat4 rotMat;
	glm_quat_mat4(quat, rotMat);

	glm_mat4_mul(transform, rotMat, transform);

	vec3 entPos;
	memcpy(entPos, brush->pos, sizeof(vec3));
	glm_mat4_mulv3(transform, entPos, 1, entPos);
	memcpy(brush->pos, entPos, sizeof(vec3));

	for (int i = 0; i < 6; i++)
	{
		Side* side = &brush->sides[i];

		for (int j = 0; j < 4; j++)
		{
			vec3 res;
			vec3 vert;
			memcpy(vert, side->verts[j], sizeof(vec3));
			glm_mat4_mulv3(transform, vert, 1, res);
			memcpy(side->verts[j], res, sizeof(vec3));
		}
	}

	vec3 entRot;
	memcpy(entRot, brush->rot, sizeof(vec3));
	entRot[0] = glm_rad(entRot[0]);
	entRot[1] = glm_rad(entRot[1]);
	entRot[2] = glm_rad(entRot[2]);
	mat4 rotMat2;
	glm_euler_yxz(entRot, rotMat2);

	glm_mat4_mul(rotMat, rotMat2, rotMat);
	getEulerAngles(rotMat, rot);
	rot[0] = glm_deg(rot[0]);
	rot[1] = glm_deg(rot[1]);
	rot[2] = glm_deg(rot[2]);
	memcpy(brush->rot, rot, sizeof(vec3));

	return 0;
}
