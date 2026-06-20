#pragma  once

#include "renderer/framebuffer.h"
#include <GLFW/glfw3.h>

void initUi(GLFWwindow* window);
void uiViewport(FrameBuffer* framebuffer);
void uiMenuBar();
void uiNewFrame();
void uiEndFrame();
