#include "item/item.h"
#include "dynList.h"
#include "error.h"
#include "export/brush.h"
#include "export/entity.h"
#include "export/script/api.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include <string.h>

static Item* item;
static char itemName[50];

// Item.getPosition(): vec3;
static int itemGetPosition(lua_State* l);
// Item.getRotation(): vec3;
static int itemGetRotation(lua_State* l);
// Item.getTransform(): mat4;
static int itemGetTransform(lua_State* l);
// Item.getName(): vec3;
static int itemGetName(lua_State* l);
// Item.getType(): vec3;
static int itemGetType(lua_State* l);
// Item.getKv(string name): (number|boolena|string), (number|string)?
static int itemGetKv(lua_State* l);

// Item.defaultExport(): Entity?, Brush[]?;
static int itemDefaultExport(lua_State* l);

// Item.getSize(): (vec2|vec3);
static int itemGetSize(lua_State* l);
// Item.getTile(): vec2;
static int itemGetTile(lua_State* l);

// Item.instanceStr(string instance): string;
static int itemInstanceStr(lua_State* l);

void addItemApi(lua_State* l, Item* i)
{
	item = i;
	lua_newtable(l);
	lua_pushcfunction(l, itemGetPosition);
	lua_setfield(l, -2, "getPosition");
	lua_pushcfunction(l, itemGetRotation);
	lua_setfield(l, -2, "getRotation");
	lua_pushcfunction(l, itemGetTransform);
	lua_setfield(l, -2, "getTransform");
	lua_pushcfunction(l, itemGetName);
	lua_setfield(l, -2, "getName");
	lua_pushcfunction(l, itemGetType);
	lua_setfield(l, -2, "getType");
	lua_pushcfunction(l, itemGetKv);
	lua_setfield(l, -2, "getKv");
	lua_pushcfunction(l, itemDefaultExport);
	lua_setfield(l, -2, "defaultExport");

	lua_pushcfunction(l, itemInstanceStr);
	lua_setfield(l, -2, "instanceStr");

	if (item->def->type != ITEM_TYPE_ENTITY)
	{
		lua_pushcfunction(l, itemGetSize);
		lua_setfield(l, -2, "getSize");
		lua_pushcfunction(l, itemGetTile);
		lua_setfield(l, -2, "getTile");
	}

	lua_setglobal(l, "Item");

	snprintf(itemName, 50, "%s%d", item->def->name, item->index);
}

int itemGetPosition(lua_State* l)
{
	luaPushVec3(l, item->pos);
	return 1;
}

int itemGetRotation(lua_State* l)
{
	luaPushVec3(l, item->dir);
	return 1;
}

int itemGetTransform(lua_State* l)
{
	mat4 mat;

	vec3 pos;
	vec3 rot;
	memcpy(pos, item->pos, sizeof(vec3));
	memcpy(rot, item->dir, sizeof(vec3));

	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

	glm_euler_yxz(rot, mat);
	mat[3][0] = pos[0];
	mat[3][1] = pos[1];
	mat[3][2] = pos[2];

	luaPushMat4(l, mat);

	return 1;
}

int itemGetName(lua_State* l)
{
	lua_pushstring(l, itemName);
	return 1;
}

int itemGetType(lua_State* l)
{
	lua_pushstring(l, item->def->name);
	return 1;
}

int itemGetKv(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");

	const char* kvName = lua_tostring(l, 1);

	int kvCount = dynList_size(item->kv);
	for (int i = 0; i < kvCount; i++)
	{
		ItemKv* kv = &item->kv[i];
		ItemKvDef* def = kv->def;
		if (strcmp(def->name, kvName))
			continue;
		int type = def->type & ~(TYPE_INSTANCE);
		if (type == TYPE_INT)
			lua_pushnumber(l, kv->value.i);
		if (type == TYPE_BOOL)
			lua_pushboolean(l, kv->value.b);
		if (type == TYPE_FLOAT)
			lua_pushnumber(l, kv->value.f);
		if (type == TYPE_STRING)
			lua_pushstring(l, kv->value.s);
		if (type == TYPE_PICKER)
		{
			if (kv->value.i == -1)
				lua_pushnil(l);
			else
			{
				Item* target = getItem(kv->value.i);
				char buf[64];
				snprintf(buf, 64, "%s%d", target->def->name, target->index);
				lua_pushstring(l, buf);
			}
		}

		if (type & TYPE_DROPDOWN)
		{
			int type2 = type & ~(TYPE_DROPDOWN);
			if (type2 == TYPE_INT)
			{
				lua_pushnumber(l, kv->value.i);
				lua_pushnumber(l, def->dropValues[kv->value.i].i);
			}
			if (type2 == TYPE_STRING)
			{
				lua_pushnumber(l, kv->value.i);
				lua_pushstring(l, def->dropValues[kv->value.i].s);
			}
			return 2;
		}
		return 1;
	}
	luaL_error(l, "invalid key name `%s`", kvName);
	return 0;
}

int itemGetSize(lua_State* l)
{
	if (item->def->type == ITEM_TYPE_PANEL)
	{
		PanelData* data = item->data;
		luaPushVec2(l, data->size);
		return 1;
	}
	if (item->def->type == ITEM_TYPE_VOLUME)
	{
		VolumeItemData* data = item->data;
		luaPushVec3(l, data->size);
		return 1;
	}
	luaL_error(l, "getSize() must be called on panel or volume item");
	return 0;
}

int itemGetTile(lua_State* l)
{
	if (item->def->type == ITEM_TYPE_PANEL)
	{
		PanelData* data = item->data;
		vec2 tile = {data->tile[0], data->tile[1]};
		luaPushVec2(l, tile);
		return 1;
	}
	luaL_error(l, "getTile() must be called on panel item");
	return 0;
}

int itemInstanceStr(lua_State* l)
{
	if (lua_type(l, 1) != LUA_TSTRING)
		luaL_error(l, "bad arg 1, expected string");

	const char* s = lua_tostring(l, 1);

	int offset = 0;
	int res = 0;
	initErrorStackD("instanceStr", res);
	if (res)
		luaL_error(l, getError());

	Package* package = processPackageString(item->def->package, s, &offset);
	lua_pushfstring(l, "pmInstances/%s/%s", package->name, s + offset);

	return 1;
}

int itemDefaultExport(lua_State* l)
{
	Entity* entityList = getEntityList();
	Brush* brushList = getBrushArray();
	int entityCount = dynList_size(entityList);
	int brushCount = dynList_size(brushList);

	item->def->callbacks->exportItem(item);

	entityList = getEntityList();
	brushList = getBrushArray();

	int newEntityCount = dynList_size(entityList);
	int newBrushCount = dynList_size(brushList);

	if (entityCount != newEntityCount)
	{
		for (int i = entityCount; i < newEntityCount; i++)
		{
			Entity* ent = &entityList[i];
			lua_newtable(l);
			luaL_getmetatable(l, "entity");
			lua_setmetatable(l, -2);

			long long id = ent->id;
			lua_pushlightuserdata(l, (void*)id);
			lua_setfield(l, -2, "id");
		}
	}
	else
		lua_pushnil(l);
	if (brushCount != newBrushCount)
	{
		lua_newtable(l);
		int j = 1;
		for (int i = brushCount; i < newBrushCount; i++)
		{
			Brush* brush = &brushList[i];
			for (int k = 0; k < 6; k++)
				brush->sides[k].material = strdup(brush->sides[k].material);
			brush->script = 1;

			long long id = brush->id;
			lua_newtable(l);
			lua_pushlightuserdata(l, (void*)id);
			lua_setfield(l, -2, "id");
			luaL_getmetatable(l, "brush");
			lua_setmetatable(l, -2);
			lua_seti(l, -2, j++);
		}
	}
	else
		lua_pushnil(l);
	return 2;
}
