#include "light.h"
#include <glad/glad.h>
#include <string.h>

static unsigned int buffer;
static int bindingPoint = 1;
static int blockIndex;

extern unsigned int shader;

#define LIGHT_COUNT 128

static Light lights[LIGHT_COUNT];

void initLights()
{
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * LIGHT_COUNT, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	blockIndex = glGetUniformBlockIndex(shader, "LightBlock");
	glUniformBlockBinding(shader, blockIndex, bindingPoint);

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
