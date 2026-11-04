#include "renderer.h"
#include "camera.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "debug.h"
#include "glad/glad.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static unsigned int shader;
static unsigned int va;
static unsigned int vb;
static unsigned int ib;

#pragma pack(push, 1)
typedef struct
{
	float pos[3];
	float uv[2];
	float tint[4];
	int mat;
} Vertex;
#pragma pack(pop)

static Vertex* verts;
static Vertex* vertBase;
static int vertCount = 0;
static int quadCount = 0;

static unsigned int textures[16];

int renderPass = 0;
static char backfaceCull = 1;
static char depthTest = 1;

#define NUM_QUADS 64
#define NUM_VERTS NUM_QUADS * 4

static mat4 camMat;
static mat4 transform;

void setRenderPass(int i)
{
	renderPass = i;
}

void setBackfaceCull(char enable)
{
	backfaceCull = enable;
}

void setDepthTest(char enable)
{
	endFrame();
	depthTest = enable;
	if (depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

mat4* getProjMat()
{
	return &projMat;
}

mat4* getCamMat()
{
	return &camMat;
}

void setCamMat(mat4 mat)
{
	memcpy(camMat, mat, sizeof(mat4));
}

void setTransform(mat4 mat)
{
	memcpy(transform, mat, sizeof(mat4));
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
	initMeshRenderer();
	initDebug();

	shader = makeShader("quad");

	glGenVertexArrays(1, &va);
	glBindVertexArray(va);

	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTS, 0, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, 10 * sizeof(float), (void*)(9 * sizeof(float)));

	int indsSize = sizeof(unsigned int) * 6 * NUM_QUADS;
	unsigned int* inds = malloc(indsSize);
	int v = 0;
	for (int i = 0; i < NUM_QUADS * 6; i += 6)
	{
		inds[i + 0] = v + 1;
		inds[i + 1] = v + 0;
		inds[i + 2] = v + 2;
		inds[i + 3] = v + 2;
		inds[i + 4] = v + 0;
		inds[i + 5] = v + 3;
		v += 4;
	}

	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indsSize, inds, GL_STATIC_DRAW);

	verts = malloc(sizeof(Vertex) * NUM_VERTS);
	vertBase = verts;
	vertCount = 0;
	quadCount = 0;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(shader);

	int texSlots[16];
	for (int i = 0; i < 16; i++)
		texSlots[i] = i;

	int loc = glGetUniformLocation(shader, "textures");
	glUniform1iv(loc, 16, texSlots);

	for (int i = 0; i < NUM_VERTS; i++)
		verts[i].mat = 0;

	for (int i = 0; i < 16; i++)
		textures[i] = 0;
}

void rendererCleanup()
{
	free(vertBase);
	glDeleteBuffers(1, &vb);
	glDeleteBuffers(1, &ib);
	glDeleteVertexArrays(1, &va);
	glDeleteProgram(shader);
}

void endFrame()
{
	if (vertCount == 0)
		return;

	// printf("end frame %d\n", quadCount);
	glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertCount, vertBase);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glUseProgram(shader);

	for (int i = 0; i < 16; i++)
	{
		if (textures[i] == 0)
			break;
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		textures[i] = 0;
	}

	setUniformMat4(shader, "cam", camMat);
	setUniformMat4(shader, "mat", projMat);
	setUniformi(shader, "pass", renderPass);

	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	verts = vertBase;
	vertCount = 0;
	quadCount = 0;
}

static unsigned int addTexture(unsigned int texture)
{
	for (int i = 0; i < 16; i++)
	{
		if (textures[i] == texture)
			return i;
		if (textures[i] == 0)
		{
			textures[i] = texture;
			return i;
		}
	}

	endFrame();
	textures[0] = texture;
	return 0;
}

static inline void setVec(vec3 v, vec3 start, float x, float y, float z)
{
	v[0] = start[0] + x;
	v[1] = start[1] + y;
	v[2] = start[2] + z;
}

// |--------------|
// |0,1        1,1|
// | 3          2 |
// |              |
// | 0          1 |
// |0,0        1,0|
// |--------------|

void genVerts(int direction, vec3 start, vec2 size, vec3* positions)
{
	float w = size[0];
	float h = size[1];
	switch (direction)
	{
	case DIR_POS_X:
		setVec(positions[0], start, 0, 0, 0);
		setVec(positions[1], start, 0, 0, w);
		setVec(positions[2], start, 0, h, w);
		setVec(positions[3], start, 0, h, 0);
		break;
	case DIR_NEG_X:
		setVec(positions[0], start, 0, 0, w);
		setVec(positions[1], start, 0, 0, 0);
		setVec(positions[2], start, 0, h, 0);
		setVec(positions[3], start, 0, h, w);
		break;
	case DIR_POS_Y:
		setVec(positions[0], start, 0, 0, 0);
		setVec(positions[1], start, w, 0, 0);
		setVec(positions[2], start, w, 0, h);
		setVec(positions[3], start, 0, 0, h);
		break;
	case DIR_NEG_Y:
		setVec(positions[0], start, w, 0, 0);
		setVec(positions[1], start, 0, 0, 0);
		setVec(positions[2], start, 0, 0, h);
		setVec(positions[3], start, w, 0, h);
		break;
	case DIR_POS_Z:
		setVec(positions[0], start, w, 0, 0);
		setVec(positions[1], start, 0, 0, 0);
		setVec(positions[2], start, 0, h, 0);
		setVec(positions[3], start, w, h, 0);
		break;
	case DIR_NEG_Z:
		setVec(positions[0], start, 0, 0, 0);
		setVec(positions[1], start, w, 0, 0);
		setVec(positions[2], start, w, h, 0);
		setVec(positions[3], start, 0, h, 0);
		break;
	}
}

void configVerts(Texture* texture)
{
	if (texture->alignment != TEX_ALIGNMENT_FIT)
	{
		vec3 uDir;
		vec3 vDir;

		uDir[0] = verts[3].pos[0] - verts[2].pos[0];
		uDir[1] = verts[3].pos[1] - verts[2].pos[1];
		uDir[2] = verts[3].pos[2] - verts[2].pos[2];

		vDir[0] = verts[1].pos[0] - verts[2].pos[0];
		vDir[1] = verts[1].pos[1] - verts[2].pos[1];
		vDir[2] = verts[1].pos[2] - verts[2].pos[2];

		float width = uDir[0] * uDir[0] + uDir[1] * uDir[1] + uDir[2] * uDir[2];
		float height = vDir[0] * vDir[0] + vDir[1] * vDir[1] + vDir[2] * vDir[2];

		width = sqrtf(width);
		height = sqrtf(height);

		width /= texture->size;
		height /= texture->size;

		verts[0].uv[0] = width;
		verts[0].uv[1] = 0;
		verts[1].uv[0] = 0;
		verts[1].uv[1] = 0;
		verts[3].uv[0] = width;
		verts[3].uv[1] = height / texture->size;
		verts[2].uv[0] = 0;
		verts[2].uv[1] = height / texture->size;
	}
	else
	{
		verts[0].uv[0] = texture->scale[0];
		verts[0].uv[1] = 0;
		verts[1].uv[0] = 0;
		verts[1].uv[1] = 0;
		verts[3].uv[0] = texture->scale[0];
		verts[3].uv[1] = texture->scale[1];
		verts[2].uv[0] = 0;
		verts[2].uv[1] = texture->scale[1];
	}
}

void drawRect(vec3* positions, Texture* texture)
{
	vec4 tint = {1, 1, 1, 1};
	drawRectTint(positions, tint, texture);
}

void drawRectTint(vec3* positions, vec4 tint, Texture* texture)
{
	unsigned int tex = addTexture(texture->tex);

	for (int i = 0; i < 4; i++)
	{
		glm_mat4_mulv3(transform, positions[i], 1, verts[i].pos);
		memcpy(verts[i].tint, tint, sizeof(float) * 4);
		verts[i].mat = tex;
	}

	configVerts(texture);

	verts += 4;

	vertCount += 4;
	quadCount++;
	if (quadCount >= NUM_QUADS)
		endFrame();
	if (!backfaceCull)
		drawRectTintInv(positions, tint, texture);
}

void drawRectTintInv(vec3* positions, vec4 tint, Texture* texture)
{
	unsigned int tex = addTexture(texture->tex);

	for (int i = 0; i < 4; i++)
	{
		memcpy(verts[i].tint, tint, sizeof(float) * 4);
		verts[i].mat = tex;
	}
	glm_mat4_mulv3(transform, positions[0], 1, verts[2].pos);
	glm_mat4_mulv3(transform, positions[1], 1, verts[1].pos);
	glm_mat4_mulv3(transform, positions[2], 1, verts[0].pos);
	glm_mat4_mulv3(transform, positions[3], 1, verts[3].pos);

	configVerts(texture);

	float a = verts[0].uv[0];
	float b = verts[0].uv[1];
	verts[0].uv[0] = verts[2].uv[0];
	verts[0].uv[1] = verts[2].uv[1];
	verts[2].uv[0] = a;
	verts[2].uv[1] = b;

	verts += 4;

	vertCount += 4;
	quadCount++;
	if (quadCount >= NUM_QUADS)
		endFrame();
}

void drawRectTile(vec3* positions, float uStart, float uEnd, float vStart, float vEnd, Texture* texture)
{
	unsigned int tex = addTexture(texture->tex);

	for (int i = 0; i < 4; i++)
	{
		glm_mat4_mulv3(transform, positions[i], 1, verts[i].pos);
		verts[i].tint[0] = 1;
		verts[i].tint[1] = 1;
		verts[i].tint[2] = 1;
		verts[i].tint[3] = 1;
		verts[i].mat = tex;
	}

	verts[0].uv[0] = uStart;
	verts[0].uv[1] = vStart;
	verts[1].uv[0] = uEnd;
	verts[1].uv[1] = vStart;
	verts[3].uv[0] = uStart;
	verts[3].uv[1] = vEnd;
	verts[2].uv[0] = uEnd;
	verts[2].uv[1] = vEnd;

	verts += 4;

	vertCount += 4;
	quadCount++;
	if (quadCount >= NUM_QUADS)
		endFrame();
	if (!backfaceCull)
		drawRectTileInv(positions, uStart, uEnd, vStart, vEnd, texture);
}

void drawRectTileInv(vec3* positions, float uStart, float uEnd, float vStart, float vEnd, Texture* texture)
{
	unsigned int tex = addTexture(texture->tex);

	for (int i = 0; i < 4; i++)
	{
		verts[i].tint[0] = 1;
		verts[i].tint[1] = 1;
		verts[i].tint[2] = 1;
		verts[i].tint[3] = 1;
		verts[i].mat = tex;
	}
	glm_mat4_mulv3(transform, positions[0], 1, verts[2].pos);
	glm_mat4_mulv3(transform, positions[1], 1, verts[1].pos);
	glm_mat4_mulv3(transform, positions[2], 1, verts[0].pos);
	glm_mat4_mulv3(transform, positions[3], 1, verts[3].pos);

	verts[2].uv[0] = uStart;
	verts[2].uv[1] = vStart;
	verts[1].uv[0] = uEnd;
	verts[1].uv[1] = vStart;
	verts[3].uv[0] = uStart;
	verts[3].uv[1] = vEnd;
	verts[0].uv[0] = uEnd;
	verts[0].uv[1] = vEnd;

	verts += 4;

	vertCount += 4;
	quadCount++;
	if (quadCount >= NUM_QUADS)
		endFrame();
}
