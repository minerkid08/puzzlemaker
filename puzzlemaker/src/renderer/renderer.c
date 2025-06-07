#include "renderer.h"
#include "cglm/types.h"
#include "glad/glad.h"
#include "renderer/shader.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int prgmId;
static unsigned int va;
static unsigned int vb;
static unsigned int ib;

static unsigned int texture;

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

void setProjMat(mat4 mat)
{
	setUniformMat4(prgmId, "mat", mat);
}

void setCamMat(mat4 mat)
{
	setUniformMat4(prgmId, "cam", mat);
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
	prgmId = makeShader();

	glUseProgram(prgmId);

	setUndformi(prgmId, "tex", 0);

	glCreateVertexArrays(1, &va);
	glBindVertexArray(va);

	glCreateBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTS, 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

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

	int error = glGetError();
	while (error)
	{
		printf("gl error: %d\n", error);
		error = glGetError();
	}

	texture = loadTexture("wall.png");
	glErrCheck();
	glActiveTexture(GL_TEXTURE0);
	glErrCheck();
	glBindTexture(GL_TEXTURE_2D, texture);
	glErrCheck();

	verts = malloc(sizeof(Vertex) * NUM_VERTS);
	vertBase = verts;
	vertCount = 0;
	quadCount = 0;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void endFrame()
{
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertCount, vertBase);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glUseProgram(prgmId);
	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, 0);

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
