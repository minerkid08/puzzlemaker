#include "assetManager.h"
#include "dynList.h"
#include "packages/package.h"
#include "renderer/renderer.h"
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
	int material;
} Vertex;

unsigned int meshShader;
unsigned int uniformBuffer;

unsigned int bindingPoint;
unsigned int uniformBlockIndex;

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
		if (zip_stat(p->zip, path, 0, &stat))
			return 0;
		size = stat.size;
		zip_file_t* file = zip_fopen(p->zip, path, 0);
		if (file == 0)
			errorf("failed to open %s", path);
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

#define glErrCheck()                                                                                                   \
	{                                                                                                                  \
		int error = glGetError();                                                                                      \
		while (error)                                                                                                  \
		{                                                                                                              \
			printf("gl error: %d, %d\n", __LINE__, error);                                                             \
			error = glGetError();                                                                                      \
		}                                                                                                              \
	}

void initMeshRenderer()
{
	meshShader = makeShader("mesh");
	glGenBuffers(1, &uniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(MeshMaterial) * 4, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	bindingPoint = 0;
	uniformBlockIndex = glGetUniformBlockIndex(meshShader, "MaterialBlock");
	glUniformBlockBinding(meshShader, uniformBlockIndex, bindingPoint);

	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
}

void loadMesh(Package* package, const char* filename, Mesh* mesh)
{
	p = package;

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
		errorf("failed to load mesh '%s'", buf);
	free(buf);

	mesh->flags = 0;

	unsigned int* textures = dynList_new(m->texture_count - 1, sizeof(unsigned int));
	mesh->textureCount = m->texture_count - 1;
	mesh->textures = textures;

	for (int i = 0; i < m->texture_count - 1; i++)
	{
		fastObjTexture* tex = &m->textures[i + 1];
		textures[i] = assetManagerLoadTexture(tex->name, &mesh->flags);
	}

	MeshMaterial* materials = dynList_new(m->material_count, sizeof(MeshMaterial));
	mesh->materialCount = m->material_count;
	mesh->materials = materials;

	for (int i = 0; i < m->material_count; i++)
	{
		fastObjMaterial* mat = &m->materials[i];
		if (mat->fallback)
			errorf("material %s is not defined", mat->name);

		MeshMaterial* meshMat = &materials[i];

		meshMat->texture = mat->map_Kd;
		meshMat->tint[0] = mat->Kd[0];
		meshMat->tint[1] = mat->Kd[1];
		meshMat->tint[2] = mat->Kd[2];
		meshMat->tint[3] = mat->d;
	}

	Vertex* vertices = malloc(sizeof(Vertex) * m->index_count);
	for (int i = 0; i < m->index_count; i++)
	{
		fastObjIndex* index = &m->indices[i];
		Vertex* vert = &vertices[i];
		memcpy(vert->pos, m->positions + 3 * index->p, sizeof(float) * 3);
		memcpy(vert->texCoord, m->texcoords + 2 * index->t, sizeof(float) * 2);
	}

	for (int i = 0; i < m->face_count; i++)
	{
		Vertex* vert = &vertices[i * 3];
		vert->material = m->face_materials[i];
		vert = &vertices[i * 3 + 1];
		vert->material = m->face_materials[i];
		vert = &vertices[i * 3 + 2];
		vert->material = m->face_materials[i];
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
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (void*)(5 * sizeof(float)));

	glBindVertexArray(0);

	free(vertices);
	free(indices);
	fast_obj_destroy(m);
}

extern int renderPass;

void renderMesh(Mesh* mesh, mat4 transform)
{
	glUseProgram(meshShader);
	char buf[32];
	for (int i = 0; i < mesh->textureCount; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mesh->textures[i]);
		snprintf(buf, 32, "textures[%d]", i);
		setUniformi(meshShader, buf, i);
	}

	glBindVertexArray(mesh->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MeshMaterial) * mesh->materialCount, mesh->materials);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	setUniformMat4(meshShader, "cam", *getCamMat());
	setUniformMat4(meshShader, "mat", *getProjMat());
	setUniformMat4(meshShader, "trans", transform);
	setUniformi(meshShader, "pass", renderPass);

	glDrawElements(GL_TRIANGLES, mesh->vertCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
