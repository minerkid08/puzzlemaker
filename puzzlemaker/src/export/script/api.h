#pragma once

#include "item/item.h"
#include <cglm/cglm.h>
#include <lua/lua.h>

void addBrushApi(lua_State* l);
void addUtilsApi(lua_State* l);
void addEntityApi(lua_State* l, Item* i);

void getVec3(lua_State* l, int pos, vec3 out, const char* arg);
