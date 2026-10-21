#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "export/script/api.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "utils.h"

// mat4.new(): mat4
static int mat4New(lua_State* l);
// mat4.newTranslate(vec3 pos): mat4
static int mat4NewTranslate(lua_State* l);
// mat4.newRotation(vec3 rot): mat4
static int mat4NewRotation(lua_State* l);
// mat4.translate(mat4 mat, vec3 pos)
static int mat4Translate(lua_State* l);
// mat4.rotate(mat4 mat, vec3 rot)
static int mat4Rotate(lua_State* l);
// mat4.mul(mat4 a, mat4 b): mat4
static int mat4Mul(lua_State* l);
// mat4.mulv3(mat4 mat, vec3 vec): vec3
static int mat4Mulv3(lua_State* l);
// mat4.getPosition(mat4 mat): vec3
static int mat4GetPosition(lua_State* l);
// mat4.getRotation(mat4 mat): vec3
static int mat4GetRotation(lua_State* l);
// mat4.getForward(mat4 mat): vec3
static int mat4GetForward(lua_State* l);
// mat4.getUp(mat4 mat): vec3
static int mat4GetUp(lua_State* l);
// mat4.getRight(mat4 mat): vec3
static int mat4GetRight(lua_State* l);

void luaGetMat4(lua_State* l, int pos, mat4 out, const char* arg)
{
	if (lua_type(l, pos) != LUA_TTABLE)
		luaL_error(l, "%s, expected mat4", arg);

	for (int i = 0; i < 16; i++)
	{
		lua_geti(l, pos, i + 1);
		if (lua_type(l, -1) != LUA_TNUMBER)
			luaL_error(l, "%s index %d to mat4, expected number", arg, i + 1);
		int x = i & 3;
		int y = i / 4;
		out[y][x] = lua_tonumber(l, -1);
		lua_pop(l, 1);
	}
}

void luaPushMat4(lua_State* l, mat4 mat)
{
	lua_newtable(l);
	luaL_getmetatable(l, "mat4");
	lua_setmetatable(l, -2);
	for (int i = 0; i < 16; i++)
	{
		int x = i & 3;
		int y = i / 4;
		lua_pushnumber(l, mat[y][x]);
		lua_seti(l, -2, i + 1);
	}
}

void luaUpdateMat4(lua_State* l, int index, mat4 mat)
{
	for (int i = 0; i < 16; i++)
	{
		int x = i & 3;
		int y = i / 4;
		lua_pushnumber(l, mat[y][x]);
		lua_seti(l, index, i + 1);
	}
}

void addMat4Api(lua_State* l)
{
	lua_newtable(l);

	luaL_newmetatable(l, "mat4");
	lua_newtable(l);
	lua_pushcfunction(l, mat4Translate);
	lua_setfield(l, -2, "translate");
	lua_pushcfunction(l, mat4Rotate);
	lua_setfield(l, -2, "rotate");
	lua_pushcfunction(l, mat4Mul);
	lua_setfield(l, -2, "mul");
	lua_pushcfunction(l, mat4Mulv3);
	lua_setfield(l, -2, "mulv3");
	lua_pushcfunction(l, mat4GetPosition);
	lua_setfield(l, -2, "getPosition");
	lua_pushcfunction(l, mat4GetRotation);
	lua_setfield(l, -2, "getRotation");
	lua_pushcfunction(l, mat4GetForward);
	lua_setfield(l, -2, "forward");
	lua_pushcfunction(l, mat4GetUp);
	lua_setfield(l, -2, "up");
	lua_pushcfunction(l, mat4GetRight);
	lua_setfield(l, -2, "right");
	lua_setfield(l, -2, "__index");
	lua_setmetatable(l, -2);

	lua_pushcfunction(l, mat4New);
	lua_setfield(l, -2, "new");
	lua_pushcfunction(l, mat4NewTranslate);
	lua_setfield(l, -2, "newTranslate");
	lua_pushcfunction(l, mat4NewRotation);
	lua_setfield(l, -2, "newRotate");
	lua_setglobal(l, "mat4");
}

static int mat4New(lua_State* l)
{
	mat4 mat;
	glm_mat4_identity(mat);
	luaPushMat4(l, mat);
	return 1;
}

static int mat4NewTranslate(lua_State* l)
{
	vec3 pos;
	luaGetVec3(l, 1, pos, "bad arg 1");

	mat4 mat;
	glm_mat4_identity(mat);
	glm_translate(mat, pos);
	luaPushMat4(l, mat);
	return 1;
}

static int mat4NewRotation(lua_State* l)
{
	vec3 rot;
	luaGetVec3(l, 1, rot, "bad arg 1");

	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

	mat4 mat;
	glm_euler_yxz(rot, mat);
	luaPushMat4(l, mat);
	return 1;
}

static int mat4Translate(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");
	vec3 pos;
	luaGetVec3(l, 2, pos, "bad arg 2");

	glm_translate(mat, pos);
	luaUpdateMat4(l, 1, mat);
	return 0;
}

static int mat4Rotate(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");
	vec3 rot;
	luaGetVec3(l, 1, rot, "bad arg 1");

	rot[0] = glm_rad(rot[0]);
	rot[1] = glm_rad(rot[1]);
	rot[2] = glm_rad(rot[2]);

	mat4 mat2;
	glm_euler_yxz(rot, mat2);
	glm_mat4_mul(mat2, mat, mat);
	luaUpdateMat4(l, 1, mat);
	return 0;
}

static int mat4Mul(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");
	mat4 mat2;
	luaGetMat4(l, 2, mat2, "bad arg 2");

	glm_mat4_mul(mat, mat2, mat);
	luaPushMat4(l, mat);
	return 1;
}

static int mat4Mulv3(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");
	vec3 vec;
	luaGetVec3(l, 2, vec, "bad arg 2");

	glm_mat4_mulv3(mat, vec, 1, vec);
	luaPushVec3(l, vec);
	return 1;
}

static int mat4GetPosition(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");

	vec3 vec;
	vec[0] = mat[3][0];
	vec[1] = mat[3][1];
	vec[2] = mat[3][2];

	luaPushVec3(l, vec);
	return 1;
}

static int mat4GetRotation(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");

	vec3 vec;
	getEulerAngles(mat, vec);

	vec[0] = glm_deg(vec[0]);
	vec[1] = glm_deg(vec[1]);
	vec[2] = glm_deg(vec[2]);

	luaPushVec3(l, vec);
	return 1;
}
static int mat4GetForward(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");

	vec3 vec;
  vec[0] = mat[2][0];
  vec[1] = mat[2][1];
  vec[2] = mat[2][2];
  luaPushVec3(l, vec);
  return 1;
}

static int mat4GetUp(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");

	vec3 vec;
  vec[0] = mat[1][0];
  vec[1] = mat[1][1];
  vec[2] = mat[1][2];
  luaPushVec3(l, vec);
  return 1;
}

static int mat4GetRight(lua_State* l)
{
	mat4 mat;
	luaGetMat4(l, 1, mat, "bad arg 1");

	vec3 vec;
  vec[0] = -mat[0][0];
  vec[1] = -mat[0][1];
  vec[2] = -mat[0][2];
  luaPushVec3(l, vec);
  return 1;
}
