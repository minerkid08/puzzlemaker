#include "light.h"
#include <glad/glad.h>
#include <string.h>

static unsigned int buffer;
static int bindingPoint = 1;
static int blockIndex;
static int meshBlockIndex;

extern unsigned int shader;
extern unsigned int meshShader;

#define LIGHT_COUNT 512

static Light lights[LIGHT_COUNT];

void initLights()
{
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * LIGHT_COUNT, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	blockIndex = glGetUniformBlockIndex(shader, "LightBlock");
	glUniformBlockBinding(shader, blockIndex, bindingPoint);
	meshBlockIndex = glGetUniformBlockIndex(meshShader, "LightBlock");
	glUniformBlockBinding(meshShader, meshBlockIndex, bindingPoint);

	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer);

	for (int i = 0; i < LIGHT_COUNT; i++)
		lights[i].position[0] = -1;
}

Light* addLight()
{
	for (int i = 0; i < LIGHT_COUNT; i++)
	{
		if (lights[i].position[0] == -1)
		{
			Light* l = &lights[i];
			memset(l, 0, sizeof(Light));
			return l;
		}
	}

	return 0;
}

void removeLight(Light* light)
{
	light->position[0] = -1;
}

void updateLightBuffer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * LIGHT_COUNT, lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
