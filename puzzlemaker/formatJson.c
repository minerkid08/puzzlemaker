#include <cjson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char** argv)
{
	FILE* file = fopen(argv[1], "rb");

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;
  fclose(file);

	cJSON* json = cJSON_Parse(data);
  free(data);

  data = cJSON_Print(json);
  file = fopen(argv[1], "wb");
  fwrite(data, 1, strlen(data), file);
  fclose(file);
  cJSON_free(json);
  free(data);
}
