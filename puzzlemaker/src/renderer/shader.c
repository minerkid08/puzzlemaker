#include "shader.h"

#include "glad/glad.h"
#include <stdio.h>
#include <stdlib.h>

const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 tint;
uniform mat4 mat;
uniform mat4 cam;
out vec2 iuv;
out vec4 itint;
void main()
{
  iuv = uv;
  itint = tint;
  gl_Position = mat * cam * pos;
}
)";

const char* fragSrc = R"(
#version 330 core
uniform sampler2D tex;
in vec2 iuv;
in vec4 itint;
out vec4 color;
void main()
{
  color = texture(tex, iuv) * itint;
}
)";

int makeShaderInt(int type, const char* src);

unsigned int makeShader()
{
	int vert = makeShaderInt(GL_VERTEX_SHADER, vertSrc);
	int frag = makeShaderInt(GL_FRAGMENT_SHADER, fragSrc);

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

int makeShaderInt(int type, const char* src)
{
	int id = glCreateShader(type);
	glShaderSource(id, 1, &src, 0);
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
