#pragma once

typedef struct
{
  int width;
  int height;
  int samples;
  char swapChainTarget;
	unsigned int id;
	unsigned int color;
	unsigned int depth;
} FrameBuffer;

void framebufferInit(FrameBuffer* framebuffer);
void framebufferDelete(FrameBuffer* framebuffer);
void framebufferRegen(FrameBuffer* framebuffer);
void framebufferResize(FrameBuffer* framebuffer, int width, int height);
void framebufferBind(FrameBuffer* framebuffer);
void framebufferUnbind(FrameBuffer* framebuffer);
