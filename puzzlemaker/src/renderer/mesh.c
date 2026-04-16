#define FAST_OBJ_IMPLEMENTATION

#include "mesh.h"
#include "cglm/types.h"
#include "obj.h"
#include "shader.h"

#include <glad/glad.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
typedef struct
{
	vec3 pos;
	vec2 texCoord;
} Vertex;

unsigned int meshShader = -1;

Mesh loadMesh(const char* filename)
{
	Mesh mesh;
	if (meshShader == -1)
		meshShader = makeShader("mesh");

	mesh.shader = meshShader;

	int l = strlen(filename) + 14;
	char* buf = malloc(l + 1);

	sprintf(buf, "assets/models/%s", filename);
	buf[l] = 0;

	fastObjMesh* m = fast_obj_read(buf);
	if (m == 0)
	{
		printf("failed to load mesh '%s'\n", buf);
		exit(1);
	}
	free(buf);

	Vertex* vertices = malloc(sizeof(Vertex) * m->index_count);

	for (int i = 0; i < m->index_count; i++)
	{
		fastObjIndex* index = &m->indices[i];
		Vertex* vert = &vertices[i];
		memcpy(vert->pos, m->positions + 3 * index->p, sizeof(float) * 3);
		memcpy(vert->texCoord, m->texcoords + 2 * index->t, sizeof(float) * 2);
	}

	int* indices = malloc(sizeof(int) * m->index_count);
	for (int i = 0; i < m->index_count; i++)
		indices[i] = i;

	mesh.vertCount = m->index_count;

  glCreateVertexArrays(1, &mesh.vertexArray);
  glBindVertexArray(mesh.vertexArray);

	glCreateBuffers(1, &mesh.vertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertBuffer);
	unsigned int sizeBytes = sizeof(Vertex) * m->index_count;
	glBufferData(GL_ARRAY_BUFFER, sizeBytes, vertices, GL_STATIC_DRAW);

	glCreateBuffers(1, &mesh.indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
	sizeBytes = m->index_count * sizeof(int);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeBytes, indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

  glBindVertexArray(0);

  free(vertices);
  free(indices);
  fast_obj_destroy(m);

	return mesh;
}
