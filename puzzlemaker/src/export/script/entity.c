#include "export/entity.h"
#include "api.h"
#include "cglm/euler.h"
#include "cglm/mat4.h"
#include "cglm/util.h"
#include "dynList.h"
#include "export/brush.h"
#include "item/item.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// Entity.new(string name, string className, vec3 pos, vec3 rot): Entity;
static int entityNew(lua_State* l);
// Entity.newGlobal(string name, string className, vec3 pos, vec3 rot): Entity;
static int entityNewGlobal(lua_State* l);
// Entity.getGlobal(string name): Entity;
static int entityGetGlobal(lua_State* l);
// Entity:setPosition(vec3 pos);
static int entitySetPosition(lua_State* l);
// Entity:setRotation(vec3 rot);
static int entitySetRotation(lua_State* l);
// Entity:transform(vec3 pos, vec3 rot);
static int entityTransform(lua_State* l);
// Entity:transformMat(mat4 transform);
static int entityTransformMat(lua_State* l);
// Entity:attachBrush(Brush brush);
static int entityAttachBrush(lua_State* l);
// Entity:setKv(string k, (string|number|boolean|Entity) v);
static int entitySetKv(lua_State* l);
// Entity:setInstanceKv(string k, (string|number|boolean|Entity) v);
static int entitySetInstanceKv(lua_State* l);
// Entity:addOutput(string output, string entity, string input, string? arg, number? delay);
static int entityAddOutput(lua_State* l);
// Entity:markAsIO();
static int entityMarkAsIO(lua_State* l);

static Item* item;
static char itemName[50];

Entity* luaGetEntity(lua_State* l, int pos, const char* arg)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_error(l, "%s, expected Entity", arg);

	lua_getfield(l, pos, "id");
	if (lua_type(l, -1) != LUA_TLIGHTUSERDATA)
		luaL_error(l, "%s, invalid Entity", arg);

	long long id = (long long)lua_touserdata(l, -1);
	lua_pop(l, 1);
	return &getEntityList()[id];
}

void addEntityApi(lua_State* l, Item* i)
{
	item = i;
	lua_newtable(l);

	luaL_newmetatable(l, "entity");
	lua_newtable(l);
	lua_pushcfunction(l, entitySetPosition);
	lua_setfield(l, -2, "setPosition");
	lua_pushcfunction(l, entitySetRotation);
	lua_setfield(l, -2, "setRotation");
	lua_pushcfunction(l, entityTransform);
	lua_setfield(l, -2, "transform");
	lua_pushcfunction(l, entityTransformMat);
	lua_setfield(l, -2, "transformMat");
	lua_pushcfunction(l, entityAttachBrush);
	lua_setfield(l, -2, "attachBrush");
	lua_pushcfunction(l, entitySetKv);
	lua_setfield(l, -2, "setKv");
	lua_pushcfunction(l, entitySetInstanceKv);
	lua_setfield(l, -2, "setInstanceKv");
	lua_pushcfunction(l, entityAddOutput);
	lua_setfield(l, -2, "addOutput");
	lua_pushcfunction(l, entityMarkAsIO);
	lua_setfield(l, -2, "markAsIO");
	lua_setfield(l, -2, "__index");

	lua_setmetatable(l, -2);
	lua_pushcfunction(l, entityNew);
	lua_setfield(l, -2, "new");
	lua_pushcfunction(l, entityNewGlobal);
	lua_setfield(l, -2, "newGlobal");
	lua_pushcfunction(l, entityGetGlobal);
	lua_setfield(l, -2, "getGlobal");

	lua_setglobal(l, "Entity");

	snprintf(itemName, 50, "%s%d", item->def->name, item->index);
}

int entityNew(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");

	vec3 pos;
	vec3 rot;
	luaGetVec3(l, 3, pos, "bad arg 3");
	luaGetVec3(l, 4, rot, "bad arg 4");
	Entity* entity = exportCreateEntity();
	char name[64];
	snprintf(name, 64, "%s-%s", itemName, lua_tostring(l, 1));
	entity->name = strdup(name);
	entity->className = strdup(lua_tostring(l, 2));
	entity->script = 1;
	memcpy(entity->pos, pos, sizeof(vec3));
	memcpy(entity->rotation, rot, sizeof(vec3));

	lua_newtable(l);
	luaL_getmetatable(l, "entity");
	lua_setmetatable(l, -2);

	long long id = entity->id;
	lua_pushlightuserdata(l, (void*)id);
	lua_setfield(l, -2, "id");
	return 1;
}

int entityNewGlobal(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");

	vec3 pos;
	vec3 rot;
	luaGetVec3(l, 3, pos, "bad arg 3");
	luaGetVec3(l, 4, rot, "bad arg 4");
	Entity* entity = exportCreateEntity();
	entity->name = strdup(lua_tostring(l, 1));
	entity->className = strdup(lua_tostring(l, 2));
	entity->script = 1;
	memcpy(entity->pos, pos, sizeof(vec3));
	memcpy(entity->rotation, rot, sizeof(vec3));

	lua_newtable(l);
	luaL_getmetatable(l, "entity");
	lua_setmetatable(l, -2);

	long long id = entity->id;
	lua_pushlightuserdata(l, (void*)id);
	lua_setfield(l, -2, "id");
	return 1;
}

int entityGetGlobal(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");

	const char* name = lua_tostring(l, 1);

	int entId = -1;
	Entity* entityList = getEntityList();
	int entityCount = dynList_size(entityList);
	for (int i = 0; i < entityCount; i++)
	{
		Entity* ent = &entityList[i];
		if (strcmp(ent->name, name) == 0)
		{
			entId = i;
		}
	}
	if (entId == -1)
		luaL_error(l, "no item with the name '%s' found\n", name);

	lua_newtable(l);
	luaL_getmetatable(l, "entity");
	lua_setmetatable(l, -2);
	long long id = entId;
	lua_pushlightuserdata(l, (void*)id);
	lua_setfield(l, -2, "id");
	return 1;
}

int entitySetPosition(lua_State* l)
{
	vec3 pos;
	luaGetVec3(l, 2, pos, "bad arg 2");
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	memcpy(entity->pos, pos, sizeof(vec3));
	return 0;
}

int entitySetRotation(lua_State* l)
{
	vec3 rot;
	luaGetVec3(l, 2, rot, "bad arg 2");
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	memcpy(entity->rotation, rot, sizeof(vec3));
	return 0;
}

int entityTransform(lua_State* l)
{
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	vec3 pos;
	vec3 rot;
	luaGetVec3(l, 2, pos, "bad arg 2");
	luaGetVec3(l, 3, rot, "bad arg 3");

	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

	mat4 transform;
	glm_euler_yxz(rot, transform);
	transform[3][0] = pos[0];
	transform[3][1] = pos[1];
	transform[3][2] = pos[2];

	memcpy(pos, entity->pos, sizeof(vec3));
	memcpy(rot, entity->rotation, sizeof(vec3));

	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

	mat4 trans2;
	glm_euler_yxz(rot, trans2);
	trans2[3][0] = pos[0];
	trans2[3][1] = pos[1];
	trans2[3][2] = pos[2];

	glm_mat4_mul(transform, trans2, transform);

	pos[0] = transform[3][0];
	pos[1] = transform[3][1];
	pos[2] = transform[3][2];

	getEulerAngles(transform, rot);
	rot[0] = glm_deg(rot[0]);
	rot[1] = glm_deg(rot[1]);
	rot[2] = glm_deg(rot[2]);
	memcpy(entity->pos, pos, sizeof(vec3));
	memcpy(entity->rotation, rot, sizeof(vec3));

	return 0;
}

static int entityTransformMat(lua_State* l)
{
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	mat4 transform;
	luaGetMat4(l, 2, transform, "bad arg 2");

	vec3 pos;
	vec3 rot;
	memcpy(pos, entity->pos, sizeof(vec3));
	memcpy(rot, entity->rotation, sizeof(vec3));
	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

	mat4 trans2;
	glm_euler_yxz(rot, trans2);
	trans2[3][0] = pos[0];
	trans2[3][1] = pos[1];
	trans2[3][2] = pos[2];

	glm_mat4_mul(transform, trans2, transform);

	pos[0] = transform[3][0];
	pos[1] = transform[3][1];
	pos[2] = transform[3][2];

	getEulerAngles(transform, rot);
	rot[0] = glm_deg(rot[0]);
	rot[1] = glm_deg(rot[1]);
	rot[2] = glm_deg(rot[2]);
	memcpy(entity->pos, pos, sizeof(vec3));
	memcpy(entity->rotation, rot, sizeof(vec3));
	return 0;
}

int entityAttachBrush(lua_State* l)
{
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	Brush* brush = luaGetBrush(l, 2, "bad arg 2");

	exportEntityAddBrush(entity, brush);
	return 0;
}

int entitySetKv(lua_State* l)
{
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 2, expected string");
	char t = lua_type(l, 3);
	if (t != LUA_TSTRING && t != LUA_TNUMBER && t != LUA_TBOOLEAN && t != LUA_TTABLE)
		luaL_error(l, "bad arg 3, expected (string|number|boolean|Entity)");

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
	case LUA_TTABLE: {
		Entity* ent = luaGetEntity(l, 3, "bad arg 3");
		snprintf(value, 50, "%s", ent->name);
		break;
	}
	}
	exportEntityAddKvss(entity, key, value);
	return 0;
}

int entitySetInstanceKv(lua_State* l)
{
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 2, expected string");
	char t = lua_type(l, 3);
	if (t != LUA_TSTRING && t != LUA_TNUMBER && t != LUA_TBOOLEAN && t != LUA_TTABLE)
		luaL_error(l, "bad arg 3, expected (string|number|boolean|Entity)");

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
	case LUA_TTABLE: {
		Entity* ent = luaGetEntity(l, 3, "bad arg 3");
		snprintf(value, 50, "%s", ent->name);
		break;
	}
	}
	ItemKv kv;
	kv.value.s = value;
	ItemKvDef def;
	def.name = key;
	def.type = TYPE_STRING | TYPE_INSTANCE;
	kv.def = &def;
	exportEntityAddKv(entity, &kv);
	return 0;
}

int entityAddOutput(lua_State* l)
{
	if (lua_gettop(l) == 4)
		lua_pushnil(l);
	if (lua_gettop(l) == 5)
		lua_pushnil(l);
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");
	if (lua_type(l, 2) != LUA_TSTRING)
		luaL_error(l, "bad arg 2, expected string");
	char type3 = lua_type(l, 3);
	if (type3 != LUA_TSTRING && type3 != LUA_TTABLE)
		luaL_error(l, "bad arg 3, expected (string|Entity)");

	if (lua_type(l, 4) != LUA_TSTRING)
		luaL_error(l, "bad arg 4, expected string");
	char type5 = lua_type(l, 5);
	if (type5 != LUA_TSTRING && type5 != LUA_TNIL)
		luaL_error(l, "bad arg 5, expected string?");
	char type6 = lua_type(l, 6);
	if (type6 != LUA_TNUMBER && type6 != LUA_TNIL)
		luaL_error(l, "bad arg 6, expected number?");

	const char* output = lua_tostring(l, 2);
	char entName[64];
	if (type3 == LUA_TTABLE)
	{
		Entity* ent2 = luaGetEntity(l, 3, "bad arg 3");
		strncpy(entName, ent2->name, 64);
	}
	else
		snprintf(entName, 64, "%s-%s", itemName, lua_tostring(l, 3));
	const char* input = lua_tostring(l, 4);
	const char* arg = "";
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
	Entity* entity = luaGetEntity(l, 1, "bad arg 1");

	item->ioEnt = strdup(entity->name);
	entity->outputs = item->outputs;
	return 0;
}
