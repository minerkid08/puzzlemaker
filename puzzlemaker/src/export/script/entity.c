#include "export/entity.h"
#include "api.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/vec4.h"
#include "export/brush.h"
#include "item/item.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "utils.h"
#include <string.h>

// Entity.new(string name, string className, vec3 pos, vec3 rot);
static int entityNew(lua_State* l);
// Entity:setPosition(vec3 pos);
static int entitySetPosition(lua_State* l);
// Entity:setRotation(vec3 rot);
static int entitySetRotation(lua_State* l);
// Entity:transform(vec3 pos, vec3 rot);
static int entityTransform(lua_State* l);
// Entity:attachBrush(Brush brush);
static int entityAttachBrush(lua_State* l);
// Entity:setKv(string k, (string|number|boolean) v);
static int entitySetKv(lua_State* l);
// Entity:addOutput(string output, string entity, string input, string? arg, number? delay);
static int entityAddOutput(lua_State* l);
// Entity:markAsIO(lua_State* l);
static int entityMarkAsIO(lua_State* l);

static Item* item;

void addEntityApi(lua_State* l, Item* i)
{
	item = i;
	lua_newtable(l);
	lua_pushcfunction(l, entityNew);
	lua_setfield(l, -2, "new");
	lua_pushcfunction(l, entitySetPosition);
	lua_setfield(l, -2, "setPosition");
	lua_pushcfunction(l, entitySetRotation);
	lua_setfield(l, -2, "setRotation");
	lua_pushcfunction(l, entityTransform);
	lua_setfield(l, -2, "transform");
	lua_pushcfunction(l, entityAttachBrush);
	lua_setfield(l, -2, "attachBrush");
	lua_pushcfunction(l, entitySetKv);
	lua_setfield(l, -2, "setKv");
	lua_pushcfunction(l, entityAddOutput);
	lua_setfield(l, -2, "addOutput");
	lua_pushcfunction(l, entityMarkAsIO);
	lua_setfield(l, -2, "markAsIO");
	lua_setglobal(l, "Entity");
}

int entityNew(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");

	vec3 pos;
	vec3 rot;
	getVec3(l, 3, pos, "bad arg 3");
	getVec3(l, 4, rot, "bad arg 4");
	Entity* entity = exportCreateEntity();
	entity->name = strdup(lua_tostring(l, 1));
	entity->className = strdup(lua_tostring(l, 2));
	entity->script = 1;
	memcpy(entity->pos, pos, sizeof(vec3));
	memcpy(entity->rotation, rot, sizeof(vec3));

	lua_newtable(l);
	lua_pushcfunction(l, entitySetPosition);
	lua_setfield(l, -2, "setPosition");
	lua_pushcfunction(l, entitySetRotation);
	lua_setfield(l, -2, "setRotation");
	lua_pushcfunction(l, entityTransform);
	lua_setfield(l, -2, "transform");
	lua_pushcfunction(l, entityAttachBrush);
	lua_setfield(l, -2, "attachBrush");
	lua_pushcfunction(l, entitySetKv);
	lua_setfield(l, -2, "setKv");
	lua_pushcfunction(l, entityAddOutput);
	lua_setfield(l, -2, "addOutput");
	lua_pushcfunction(l, entityMarkAsIO);
	lua_setfield(l, -2, "markAsIO");
	long long id = entity->id;
	lua_pushlightuserdata(l, (void*)id);
	lua_setfield(l, -2, "id");
	return 1;
}

int entitySetPosition(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	vec3 pos;
	getVec3(l, 2, pos, "bad arg 2");
	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long id = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);
	Entity* entity = &getEntityList()[id];
	memcpy(entity->pos, pos, sizeof(vec3));
	return 0;
}

int entitySetRotation(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	vec3 rot;
	getVec3(l, 2, rot, "bad arg 2");
	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long id = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);
	Entity* entity = &getEntityList()[id];
	memcpy(entity->rotation, rot, sizeof(vec3));
	return 0;
}

int entityTransform(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	vec3 pos;
	vec3 rot;
	getVec3(l, 2, pos, "bad arg 2");
	getVec3(l, 2, rot, "bad arg 3");
  
	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long entId = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);

	Entity* entity = &getEntityList()[entId];

	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

  mat4 transform;
  mat4 rotMat;
  glm_translate(transform, pos);
  glm_euler_yzx(rot, rotMat);
	glm_mat4_mul(transform, rotMat, transform);

  vec3 entPos;
  memcpy(entPos, entity->pos, sizeof(vec3));
  glm_mat4_mulv3(transform, entPos, 1, entPos);
  memcpy(entity->pos, entPos, sizeof(vec3));

  vec3 entRot;
  memcpy(entRot, entity->rotation, sizeof(vec3));
  mat4 rotMat2;
  glm_euler_yzx(entRot, rotMat2);

  glm_mat4_mul(rotMat2, rotMat, rotMat);
  getEulerAngles(rotMat, rot);
	rot[0] = glm_deg(rot[0]);
	rot[1] = glm_deg(rot[1]);
	rot[2] = glm_deg(rot[2]);
  memcpy(entity->rotation, rot, sizeof(vec3));
  return 0;
}

int entityAttachBrush(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	if (lua_type(l, 2) != LUA_TTABLE)
		luaL_error(l, "bad arg 2, expected Brush");

	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long entId = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, 2, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 2, invalid Entity");
	long long brushId = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);

	Entity* entity = &getEntityList()[entId];
	Brush* brush = &getBrushArray()[brushId];

	exportEntityAddBrush(entity, brush);
	return 0;
}

int entitySetKv(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 2, expected string");
	char t = lua_type(l, 3);
	if (t != LUA_TSTRING && t != LUA_TNUMBER && t != LUA_TBOOLEAN)
		luaL_error(l, "bad arg 3, expected (string|number|boolean)");

	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long entId = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);

	Entity* entity = &getEntityList()[entId];

	const char* key = lua_tostring(l, 2);

	char value[50];
	switch (t)
	{
	case LUA_TNUMBER: {
		double v = lua_tonumber(l, 3);
		snprintf(value, 50, "%.2f", v);
		break;
	}
	case LUA_TSTRING: {
		const char* v = lua_tostring(l, 3);
		snprintf(value, 50, "%s", v);
		break;
	}
	case LUA_TBOOLEAN: {
		char v = lua_toboolean(l, 3);
		snprintf(value, 50, "%d", v);
		break;
	}
	}
	exportEntityAddKvss(entity, key, value);
	return 0;
}

int entityAddOutput(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 2, expected string");
	if (lua_type(l, 3) != LUA_TSTRING)
		luaL_error(l, "bad arg 3, expected string");
	if (lua_type(l, 4) != LUA_TSTRING)
		luaL_error(l, "bad arg 4, expected string");
	char type5 = lua_type(l, 5);
	if (type5 != LUA_TSTRING && type5 != LUA_TNIL)
		luaL_error(l, "bad arg 5, expected string?");
	char type6 = lua_type(l, 6);
	if (type6 != LUA_TNUMBER && type6 != LUA_TNIL)
		luaL_error(l, "bad arg 6, expected number?");

	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long entId = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);

	Entity* entity = &getEntityList()[entId];

	const char* output = lua_tostring(l, 2);
	const char* entName = lua_tostring(l, 3);
	const char* input = lua_tostring(l, 4);
	const char* arg = 0;
	float delay = 0;
	if (type5 == LUA_TSTRING)
		arg = lua_tostring(l, 5);
	if (type6 == LUA_TNUMBER)
		delay = lua_tonumber(l, 6);

	exportEntityAddRawOutput(entity, output, entName, input, arg, delay);
	return 0;
}

int entityMarkAsIO(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TTABLE)
		luaL_error(l, "bad arg 1, expected Entity");
	lua_getfield(l, 1, "id");
	if (lua_type(l, -1) != LUA_TUSERDATA)
		luaL_error(l, "bad arg 1, invalid Entity");
	long long entId = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);

	Entity* entity = &getEntityList()[entId];

	item->ioEnt = strdup(entity->name);
	entity->outputs = item->outputs;
	return 0;
}
