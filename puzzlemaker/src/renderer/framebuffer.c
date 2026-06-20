#include <glad/glad.h>
#include <stdio.h>
#include "renderer/framebuffer.h"

void framebufferInit(FrameBuffer* framebuffer)
{
  framebuffer->id = 0;
  framebufferRegen(framebuffer);
}

void framebufferDelete(FrameBuffer* framebuffer)
{
	if (framebuffer->id)
	{
		framebuffer->id = 0;
		glDeleteFramebuffers(1, &framebuffer->id);
		glDeleteTextures(1, &framebuffer->color);
		glDeleteTextures(1, &framebuffer->depth);
	}
}

void framebufferRegen(FrameBuffer* framebuffer)
{
	if (framebuffer->id)
	{
		glDeleteFramebuffers(1, &framebuffer->id);
		glDeleteTextures(1, &framebuffer->color);
		glDeleteTextures(1, &framebuffer->depth);
	}
	glGenFramebuffers(1, &framebuffer->id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
	glGenTextures(1, &framebuffer->color);
	glBindTexture(GL_TEXTURE_2D, framebuffer->color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, framebuffer->width, framebuffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &framebuffer->depth);
	glBindTexture(GL_TEXTURE_2D, framebuffer->depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, framebuffer->width, framebuffer->height, 0, GL_DEPTH_STENCIL,
				 GL_UNSIGNED_INT_24_8, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer->depth, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
    printf("framebuffer could not be created");
	}
	framebufferUnbind(framebuffer);
}
void framebufferResize(FrameBuffer* framebuffer, int width, int height)
{
	framebuffer->width = width;
	framebuffer->height = height;
	framebufferRegen(framebuffer);
}
void framebufferBind(FrameBuffer* framebuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
}
void framebufferUnbind(FrameBuffer* framebuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
