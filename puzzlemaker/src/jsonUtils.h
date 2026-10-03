#include "cglm/types.h"
#include <cjson.h>

char* jsonGetStr(const cJSON* json, const char* name);
char jsonGetBool(const cJSON* json, const char* name);
float jsonGetFloat(const cJSON* json, const char* name);
int jsonGetInt(const cJSON* json, const char* name);
void jsonGetVec2(const cJSON* json, const char* name, vec2 out);
void jsonGetVec3(const cJSON* json, const char* name, vec3 out);

char jsonGetBoolC(const cJSON* json, const char* name, char def);

char* jsonArrGetStr(const cJSON* json, int i);
char jsonArrGetBool(const cJSON* json, int i);
float jsonArrGetFloat(const cJSON* json, int i);
int jsonArrGetInt(const cJSON* json, int i);

void jsonArrSetStr(cJSON* arr, const char* s);
void jsonArrSetBool(cJSON* arr, char b);
void jsonArrSetFloat(cJSON* arr, float f);
void jsonArrSetInt(cJSON* arr, int i);

void jsonSetVec3(cJSON* json, const char* name, vec3 vec);
