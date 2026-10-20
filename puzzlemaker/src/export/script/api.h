#pragma once

#include "export/brush.h"
#include "export/entity.h"
#include "item/item.h"
#include <cglm/cglm.h>
#include <lua/lua.h>

void addBrushApi(lua_State* l);
void addUtilsApi(lua_State* l);
void addEntityApi(lua_State* l, Item* i);
void addItemApi(lua_State* l, Item* i);
void addVec2Api(lua_State* l);
void addVec3Api(lua_State* l);
void addMat4Api(lua_State* l);

void luaGetVec3(lua_State* l, int pos, vec3 out, const char* arg);
void luaGetVec2(lua_State* l, int pos, vec2 out, const char* arg);
void luaGetMat4(lua_State* l, int pos, mat4 out, const char* arg);

void luaPushVec3(lua_State* l, vec3 vec);
void luaPushVec2(lua_State* l, vec2 vec);
void luaPushiVec2(lua_State* l, ivec2 vec);
void luaPushMat4(lua_State* l, mat4 mat);

void luaUpdateMat4(lua_State* l, int index, mat4 mat);

Entity* luaGetEntity(lua_State* l, int pos, const char* arg);
Brush* luaGetBrush(lua_State* l, int pos, const char* arg);
