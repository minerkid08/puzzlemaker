#include "packages/package.h"
#include "utils.h"
#include <stdio.h>
#include <zip.h>
#define FAST_OBJ_IMPLEMENTATION

#include "cglm/types.h"
#include "mesh.h"
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

static int size;

static Package* p;

void* fileOpen(const char* path, void* user_data)
{
	if (p->zip == 0)
	{
		char buf[128];
		snprintf(buf, 128, "packages/%s/%s", p->name, path);

		FILE* file = fopen(buf, "rb");
		return file;
	}
	else
	{
		zip_stat_t stat;
		if(zip_stat(p->zip, path, 0, &stat))
      return 0;
		size = stat.size;
		zip_file_t* file = zip_fopen(p->zip, path, 0);
		if (file == 0)
			errorf("failed to open %s\n", path);
		return file;
	}
}

void fileClose(void* file, void* user_data)
{
	if (p->zip == 0)
		fclose(file);
	else
		zip_fclose(file);
}

size_t fileRead(void* file, void* dst, size_t bytes, void* user_data)
{
	if (p->zip == 0)
		return fread(dst, 1, bytes, file);
	else
		return zip_fread(file, dst, bytes);
}

unsigned long fileSize(void* file, void* user_data)
{
	if (p->zip == 0)
	{
		int p = ftell(file);
		fseek(file, 0, SEEK_END);
		int n = ftell(file);
		fseek(file, p, SEEK_SET);
		return n;
	}
	else
		return size;
}

void loadMesh(Package* package, const char* filename, Mesh* mesh)
{
  p = package;
	if (meshShader == -1)
		meshShader = makeShader("mesh");

	mesh->shader = meshShader;

	int l = strlen(filename) + 14;
	char* buf = malloc(l + 1);

	sprintf(buf, "assets/models/%s", filename);
	buf[l] = 0;

	fastObjCallbacks callbacks;
	callbacks.file_open = fileOpen;
	callbacks.file_close = fileClose;
	callbacks.file_read = fileRead;
	callbacks.file_size = fileSize;

	fastObjMesh* m = fast_obj_read_with_callbacks(buf, &callbacks, 0);
	if (m == 0)
		errorf("failed to load mesh '%s'\n", buf);
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

	mesh->vertCount = m->index_count;

	glGenVertexArrays(1, &mesh->vertexArray);
	glBindVertexArray(mesh->vertexArray);

	glGenBuffers(1, &mesh->vertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBuffer);
	unsigned int sizeBytes = sizeof(Vertex) * m->index_count;
	glBufferData(GL_ARRAY_BUFFER, sizeBytes, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &mesh->indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
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
}
