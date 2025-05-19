#include "renderer.h"
#include "cglm/types.h"
#include "glad/glad.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 ipos;
layout(location = 1) in vec2 iuv;
uniform mat4 mat;
out vec2 ouv;
void main()
{
  ouv = iuv;
  gl_Position = mat * vec4(ipos, 1);
}
)";

static const char* fragSrc = R"(
#version 330 core
in vec2 ouv;
out vec4 color;
uniform sampler2D tex;
void main()
{
  color = texture(tex, ouv);
}
)";

typedef struct {
  vec3 pos;
  vec2 uv;
} Vertex;

static int makeShader(int type, const char* src);

static unsigned int prgmId;
static unsigned int va;
static unsigned int vb;
static unsigned int ib;

static unsigned int uniformId;
static unsigned int texId;

static unsigned int texture;

void setMat(mat4 mat)
{
	glUniformMatrix4fv(uniformId, 1, GL_FALSE, (float*)mat);
}

void initRenderer()
{
	int vert = makeShader(GL_VERTEX_SHADER, vertSrc);
	int frag = makeShader(GL_FRAGMENT_SHADER, fragSrc);

	prgmId = glCreateProgram();
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
		return;
	}
//	glDeleteShader(vert);
//	glDeleteShader(frag);
//
  glUseProgram(prgmId);

	uniformId = glGetUniformLocation(prgmId, "mat");
	texId = glGetUniformLocation(prgmId, "tex");

  glUniform1i(texId, 0);
   
	glCreateVertexArrays(1, &va);
	glBindVertexArray(va);

	glCreateBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 4, 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int inds[] = {0, 1, 2, 2, 1, 3};

	glCreateBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, inds, GL_STATIC_DRAW);

	int error = glGetError();
	while (error)
	{
		printf("gl error: %d\n", error);
		error = glGetError();
	}

  texture = loadTexture("wall.png");
}

void drawVerts(const vec3* positions)
{
  Vertex verts[4];

  for(int i = 0; i < 4; i++)
  {
    memcpy(verts[i].pos, positions[i], sizeof(float) * 3);
  }
  verts[0].uv[0] = 0;
  verts[0].uv[1] = 0;
  verts[1].uv[0] = 0;
  verts[1].uv[1] = 1;
  verts[2].uv[0] = 1;
  verts[2].uv[1] = 0;
  verts[3].uv[0] = 1;
  verts[3].uv[1] = 1;
 
  glBindTextureUnit(texture, GL_TEXTURE0);

	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * 4, verts);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glUseProgram(prgmId);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	int error = glGetError();
	while (error)
	{
		printf("gl error: %d\n", error);
		error = glGetError();
	}
}

int makeShader(int type, const char* src)
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
