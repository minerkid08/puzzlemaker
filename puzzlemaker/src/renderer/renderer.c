#include "renderer.h"
#include "cglm/types.h"
#include "glad/glad.h"
#include "renderer/shader.h"
#include "debug.h"
#include <stdlib.h>
#include <string.h>

static unsigned int prgmId;
static unsigned int va;
static unsigned int vb;
static unsigned int ib;

#pragma pack(1)
typedef struct
{
	vec4 pos;
	vec2 uv;
	vec4 tint;
} Vertex;

static Vertex* verts;
static Vertex* vertBase;
static int vertCount = 0;
static int quadCount = 0;

#define NUM_QUADS 64
#define NUM_VERTS NUM_QUADS * 4

static mat4 camMat;
static mat4 projMat;

mat4* getProjMat()
{
  return &projMat;
}

mat4* getCamMat()
{
  return &camMat;
}

void setProjMat(mat4 mat)
{
  memcpy(projMat, mat, sizeof(mat4));
}

void setCamMat(mat4 mat)
{
  memcpy(camMat, mat, sizeof(mat4));
}

#define glErrCheck()                                                                                                   \
	{                                                                                                                  \
		int error = glGetError();                                                                                      \
		while (error)                                                                                                  \
		{                                                                                                              \
			printf("gl error: %d, %d\n", __LINE__, error);                                                             \
			error = glGetError();                                                                                      \
		}                                                                                                              \
	}

void initRenderer()
{
  initDebug();

	prgmId = makeShader("voxel");

	glCreateVertexArrays(1, &va);
	glBindVertexArray(va);

	glCreateBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTS, 0, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));

	int indsSize = sizeof(unsigned int) * 6 * NUM_QUADS;
	unsigned int* inds = malloc(indsSize);
	int v = 0;
	for (int i = 0; i < NUM_QUADS * 6; i += 6)
	{
		inds[i + 0] = v + 0;
		inds[i + 1] = v + 1;
		inds[i + 2] = v + 2;
		inds[i + 3] = v + 2;
		inds[i + 4] = v + 1;
		inds[i + 5] = v + 3;
		v += 4;
	}

	glCreateBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indsSize, inds, GL_STATIC_DRAW);

	verts = malloc(sizeof(Vertex) * NUM_VERTS);
	vertBase = verts;
	vertCount = 0;
	quadCount = 0;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void bindTexture(unsigned int texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void endFrame()
{
  glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertCount, vertBase);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glUseProgram(prgmId);

	setUndformi(prgmId, "tex", 0);
	setUniformMat4(prgmId, "cam", camMat);
	setUniformMat4(prgmId, "mat", projMat);

	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
	verts = vertBase;
	vertCount = 0;
	quadCount = 0;
}

void drawVerts(vec3* positions, vec4 tint)
{
	for (int i = 0; i < 4; i++)
	{
		memcpy(verts[i].pos, positions[i], sizeof(float) * 3);
		memcpy(verts[i].tint, tint, sizeof(float) * 4);
		verts[i].pos[3] = 1.0f;
	}

	verts[0].uv[0] = 0;
	verts[0].uv[1] = 0;
	verts[1].uv[0] = 0;
	verts[1].uv[1] = 1;
	verts[2].uv[0] = 1;
	verts[2].uv[1] = 0;
	verts[3].uv[0] = 1;
	verts[3].uv[1] = 1;

	verts += 4;

	vertCount += 4;
	quadCount++;
	if (quadCount >= NUM_QUADS)
		endFrame();
}

void drawMesh(Mesh *mesh, unsigned int texture, mat4 transform)
{
  bindTexture(texture);
  glBindVertexArray(mesh->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glUseProgram(mesh->shader);

	setUndformi(mesh->shader, "tex", 0);
	setUniformMat4(mesh->shader, "cam", camMat);
	setUniformMat4(mesh->shader, "mat", projMat);
	setUniformMat4(mesh->shader, "trans", transform);

	glDrawElements(GL_TRIANGLES, mesh->vertCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
