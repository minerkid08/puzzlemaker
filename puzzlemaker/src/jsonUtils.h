#include <cjson.h>

const char* jsonGetStr(const cJSON* json, const char* name);
char jsonGetBool(const cJSON* json, const char* name);
float jsonGetFloat(const cJSON* json, const char* name);
int jsonGetInt(const cJSON* json, const char* name);

const char* jsonArrGetStr(const cJSON* json, int i);
char jsonArrGetBool(const cJSON* json, int i);
float jsonArrGetFloat(const cJSON* json, int i);
int jsonArrGetInt(const cJSON* json, int i);

void jsonArrSetStr(cJSON* arr, const char* s);
void jsonArrSetBool(cJSON* arr, char b);
void jsonArrSetFloat(cJSON* arr, float f);
void jsonArrSetInt(cJSON* arr, int i);
