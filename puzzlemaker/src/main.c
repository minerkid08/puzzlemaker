#include "cglm/vec3.h"
#include <math.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cimgui.h"
#include "renderer/debug.h"
#include "renderer/renderer.h"
#include <assert.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string.h>

#include "camera.h"
#include "compileThread.h"
#include "item.h"
#include "ui.h"
#include "ui/itemPanel.h"
#include "voxel.h"
#include "voxelModification.h"

#define MODE_NONE 0
#define MODE_ORBIT 1
#define MODE_PAN 2

int width = 1920;
int height = 1080;

double tx;
double ty;

double mx;
double my;

vec3 tempPos;
vec3 tempRot;
vec4 mouseDir;

char windowHovered = 0;

Item** pickPtr = 0;

ImGuiIO* io;

void findSelected()
{
	currentVoxel = 0;
	RaycastHit hit;
	if (voxelRaycast(cameraPos, mouseDir, 10, &hit))
	{
		currentVoxel = hit.voxel;
		currentDir = hit.dir;
		memcpy(currentVoxelPos, hit.pos, sizeof(int) * 3);
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double x, double y);
void mouseZoomCallback(GLFWwindow* window, double x, double y);
void windowResizeCallback(GLFWwindow* window, int w, int h);

int main()
{
	startCompileThread();
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "puzzlemaker", 0, 0);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);


	initVoxels();

	initRenderer();
	aspect = (float)width / (float)height;
	initCamera();

	initItems();

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseZoomCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	glfwSwapInterval(1);

	initUi(window);
	initItemPanel();

	io = igGetIO_Nil();

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	double now = glfwGetTime();
	double dt;
	double lastTime = now;
	while (!glfwWindowShouldClose(window))
	{
		now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		float moveForward = glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S);
		float moveRight = glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A);

		float lookUp = glfwGetKey(window, GLFW_KEY_UP) - glfwGetKey(window, GLFW_KEY_DOWN);
		float lookRight = glfwGetKey(window, GLFW_KEY_RIGHT) - glfwGetKey(window, GLFW_KEY_LEFT);

		cameraRot[0] += lookUp * dt;
		cameraRot[1] -= lookRight * dt;

		updateCamera();

		cameraPos[0] += forward[0] * moveForward * dt * 2;
		cameraPos[1] += forward[1] * moveForward * dt * 2;
		cameraPos[2] += forward[2] * moveForward * dt * 2;

		cameraPos[0] += right[0] * moveRight * dt * 2;
		cameraPos[1] += right[1] * moveRight * dt * 2;
		cameraPos[2] += right[2] * moveRight * dt * 2;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawVoxels(cameraPos, cameraRot);
		endFrame();

		drawItems();

		uiNewFrame();

		itemPanelRender();

		uiEndFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (io->WantCaptureMouse)
		return;
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_1)
			voxelPush();
		if (key == GLFW_KEY_2)
			voxelPull();
		if (key == GLFW_KEY_3)
			voxelTogglePortal();
	}
}

int mouseMode = 0;

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (io->WantCaptureMouse)
		return;
	if (action == GLFW_PRESS)
	{
		tx = mx;
		ty = my;

		if (button == GLFW_MOUSE_BUTTON_2)
		{
			mouseMode = MODE_ORBIT;
			memcpy(tempRot, cameraRot, sizeof(float) * 3);
		}

		if (button == GLFW_MOUSE_BUTTON_3)
		{
			mouseMode = MODE_PAN;
			memcpy(tempPos, cameraPos, sizeof(float) * 3);
		}
		if (button == GLFW_MOUSE_BUTTON_1)
		{
			vec4 dir;
			dir[0] = mx / width - 0.5f;
			dir[1] = -my / height + 0.5f;

			float vertAngle = 0.5f * fovy;

			float worldHeight = 2.0f * tanf(vertAngle);

			dir[0] *= worldHeight;
			dir[1] *= worldHeight;
			dir[2] = -1;
			dir[3] = 1;

			dir[0] *= aspect;

			mat4 rotMat;
			glm_mat4_identity(rotMat);
			glm_rotate_z(rotMat, cameraRot[2], rotMat);
			glm_rotate_y(rotMat, cameraRot[1], rotMat);
			glm_rotate_x(rotMat, cameraRot[0], rotMat);

			glm_mat4_mulv(rotMat, dir, mouseDir);

			currentVoxel = 0;
			Item* item = findSelectedItem(cameraPos, mouseDir, 10);
			if (pickPtr)
			{
				*pickPtr = item;
				pickPtr = 0;
			}
			else if (item)
				setSelectedItem(item);
			else
			{
				setSelectedItem(0);
				findSelected();
			}
		}
	}
	if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_2)
			mouseMode = 0;

		if (button == GLFW_MOUSE_BUTTON_3)
			mouseMode = 0;
	}
}

void mouseMoveCallback(GLFWwindow* window, double x, double y)
{
	if (io->WantCaptureMouse)
		return;
	mx = x;
	my = y;

	double dx = tx - x;
	double dy = ty - y;

	if (mouseMode == MODE_ORBIT)
	{
		cameraRot[1] = tempRot[1] + (dx * 0.001);
		cameraRot[0] = tempRot[0] + (dy * 0.001);
	}

	if (mouseMode == MODE_PAN)
	{
		cameraPos[0] = tempPos[0] + right[0] * dx * 0.005;
		cameraPos[1] = tempPos[1] + right[1] * dx * 0.005;
		cameraPos[2] = tempPos[2] + right[2] * dx * 0.005;

		cameraPos[0] -= up[0] * dy * 0.005;
		cameraPos[1] -= up[1] * dy * 0.005;
		cameraPos[2] -= up[2] * dy * 0.005;
	}
}

void mouseZoomCallback(GLFWwindow* window, double x, double y)
{
	if (io->WantCaptureMouse)
		return;
	cameraPos[0] += forward[0] * y * 0.1;
	cameraPos[1] += forward[1] * y * 0.1;
	cameraPos[2] += forward[2] * y * 0.1;
}

void windowResizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	aspect = (float)w / (float)h;
	initCamera();
}
