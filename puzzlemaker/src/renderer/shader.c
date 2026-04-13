#include "shader.h"

#include "glad/glad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int makeShaderInt(int type, const char* src);

unsigned int makeShader(const char* name)
{
  int len = strlen(name) + 14;

  char* buf = malloc(len + 1);

  sprintf(buf, "shaders/%s.vert", name);
	int vert = makeShaderInt(GL_VERTEX_SHADER, buf);

  sprintf(buf, "shaders/%s.frag", name);
	int frag = makeShaderInt(GL_FRAGMENT_SHADER, buf);

  free(buf);

	int prgmId = glCreateProgram();
	glAttachShader(prgmId, vert);
	glAttachShader(prgmId, frag);
	glLinkProgram(prgmId);

	int status;
	glGetProgramiv(prgmId, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("shader linking failed\n");
		glDeleteShader(vert);
		glDeleteShader(frag);
		exit(1);
	}
	return prgmId;
}

void setUniformMat4(unsigned int id, const char* name, mat4 value)
{
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
	{
		printf("uniform %s no exist\n", name);
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)value);
}

void setUndformi(unsigned int id, const char* name, int value)
{
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
	{
		printf("uniform %s no exist\n", name);
		return;
	}
	glUniform1i(loc, value);
}

int makeShaderInt(int type, const char* filename)
{
  FILE* file = fopen(filename, "rb");

  fseek(file, 0, SEEK_END);
  int l = ftell(file);
  fseek(file, 0, SEEK_SET);
  
  char* buf = malloc(l + 1);
  fread(buf, 1, l, file);
  fclose(file);
  buf[l] = 0;

	int id = glCreateShader(type);
	glShaderSource(id, 1, (const char**)&buf, 0);
  free(buf);
	glCompileShader(id);
	int state;
	glGetShaderiv(id, GL_COMPILE_STATUS, &state);
	if (state == GL_FALSE)
	{
		int len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		char* err = malloc(len);
		glGetShaderInfoLog(id, len, &len, err);
		printf("shader err: %s\n", err);
		free(err);
		glDeleteShader(id);
		return -1;
	}
	return id;
}
