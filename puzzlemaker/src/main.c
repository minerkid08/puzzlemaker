#include "cglm/quat.h"
#include "cglm/util.h"
#include "renderer/framebuffer.h"
#include "ui/fileBrowser.h"
#include "utils.h"
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cglm/mat4.h"
#include "cglm/types.h"
#include "renderer/debug.h"
#include "renderer/renderer.h"
#include <assert.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string.h>

#include "camera.h"
#include "compile/compileThread.h"
#include "item/item.h"
#include "picker.h"
#include "raycast.h"
#include "selection.h"
#include "ui.h"
#include "ui/itemPanel.h"
#include "voxel/voxel.h"
#include "voxel/voxelModification.h"

Picker picker;

#define MODE_NONE 0
#define MODE_ORBIT 1
#define MODE_PAN 2
#define MODE_SELECT 3
#define MODE_GRAB 4
#define MODE_ROTATE 5

int mouseMode = 0;

#define RAY_LEN 40

int width = 1920;
int height = 1080;

double tx;
double ty;

double mx;
double my;

vec3 tempPos;
vec3 tempRot;
vec4 mouseDir;

char viewportHovered = 0;
char mouseOffsetX = 0;
char mouseOffsetY = 0;

Item** pickPtr = 0;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double x, double y);
void mouseZoomCallback(GLFWwindow* window, double x, double y);

extern float uiScale;

int main()
{
	DIR* dir = opendir("maps");
	if (dir)
		closedir(dir);
	else
		mkdir("maps", 0777);

	startCompileThread();
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "puzzlemaker", 0, 0);
	glfwMakeContextCurrent(window);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	glfwGetMonitorContentScale(monitor, &uiScale, 0);
	uiScale *= 1.4;

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	initVoxels();

	initRenderer();
	aspect = (float)width / (float)height;
	initCamera();

	loadItemDefinitions();

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseZoomCallback);

	glfwSwapInterval(1);

	initUi(window);
	initItemPanel();

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	FrameBuffer framebuffer;
	framebuffer.width = 1920;
	framebuffer.height = 1080;
	framebufferInit(&framebuffer);

	double now = glfwGetTime();
	double dt;
	double lastTime = now;
	while (!glfwWindowShouldClose(window))
	{
		framebufferBind(&framebuffer);

		now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		float moveForward = glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S);
		float moveRight = glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A);

		float lookUp = glfwGetKey(window, GLFW_KEY_UP) - glfwGetKey(window, GLFW_KEY_DOWN);
		float lookRight = glfwGetKey(window, GLFW_KEY_RIGHT) - glfwGetKey(window, GLFW_KEY_LEFT);

		if (!viewportHovered)
		{
			moveForward = 0;
			moveRight = 0;
			lookUp = 0;
			lookRight = 0;
		}

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
		framebufferUnbind(&framebuffer);

		uiNewFrame();

		uiMenuBar();
		uiViewport(&framebuffer);
		fileBrowserRender();

		itemPanelRender();

		uiEndFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

int mouseX = 0;
vec4 itemQuat;
extern Item* selectedItem;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!viewportHovered)
		return;
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_Q)
			voxelPush();
		if (key == GLFW_KEY_E)
			voxelPull();
		if (key == GLFW_KEY_R)
			voxelTogglePortal();
		if (key == GLFW_KEY_G)
			mouseMode = MODE_GRAB;
		if (key == GLFW_KEY_F)
		{
			if (selectedItem->snapDir == DIR_NONE)
				return;
			mouseMode = MODE_ROTATE;
			mouseX = mx;
			memcpy(itemQuat, selectedItem->quat, sizeof(vec4));
		}
	}
}

void calcSelectAxis()
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
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (!viewportHovered)
		return;
	if (action == GLFW_PRESS)
	{
		tx = mx;
		ty = my;
		if (mouseMode != MODE_NONE)
		{
			mouseMode = MODE_NONE;
			return;
		}

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
			calcSelectAxis();
			beginSelection(mouseDir);
		}
	}
	if (action == GLFW_RELEASE)
	{
		endSelection();
		mouseMode = 0;
	}
}

void mouseMoveCallback(GLFWwindow* window, double x, double y)
{
	if (!viewportHovered)
		return;
	double x2 = floor(x) - mouseOffsetX;
	double y2 = floor(y) - mouseOffsetY;
	mx = x2;
	my = y2;

	double dx = tx - x2;
	double dy = ty - y2;

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

	if (mouseMode == MODE_GRAB)
	{
		RaycastHit hit;
		calcSelectAxis();
		if (raycast(cameraPos, mouseDir, 40, RAYCAST_VOXEL, &hit))
		{
			float startX = hit.pos[0];
			float startY = hit.pos[1];
			float startZ = hit.pos[2];
			switch (selectedItem->def->snapMode)
			{
			case SNAP_CORNER:
				selectedItem->pos[0] = round(hit.pos[0]);
				selectedItem->pos[1] = round(hit.pos[1]);
				selectedItem->pos[2] = round(hit.pos[2]);
				break;
			case SNAP_CENTER:
				selectedItem->pos[0] = round(hit.pos[0] - 0.5) + 0.5;
				selectedItem->pos[1] = round(hit.pos[1] - 0.5) + 0.5;
				selectedItem->pos[2] = round(hit.pos[2] - 0.5) + 0.5;
				break;
			case SNAP_MINI_CORNER:
				selectedItem->pos[0] = round(hit.pos[0] * 2) / 2;
				selectedItem->pos[1] = round(hit.pos[1] * 2) / 2;
				selectedItem->pos[2] = round(hit.pos[2] * 2) / 2;
				break;
			case SNAP_MINI_CENTER:
				selectedItem->pos[0] = round(hit.pos[0] * 2 - 0.25) / 2 + 0.25;
				selectedItem->pos[1] = round(hit.pos[1] * 2 - 0.25) / 2 + 0.25;
				selectedItem->pos[2] = round(hit.pos[2] * 2 - 0.25) / 2 + 0.25;
				break;
			}
			switch (selectedItem->snapDir)
			{
			case DIR_POS_Z:
			case DIR_NEG_Z:
				selectedItem->pos[2] = startZ;
				break;
			case DIR_POS_Y:
			case DIR_NEG_Y:
				selectedItem->pos[1] = startY;
				break;
			case DIR_POS_X:
			case DIR_NEG_X:
				selectedItem->pos[0] = startX;
				break;
			}

			if (selectedItem->snapDir != hit.dir)
			{
				selectedItem->snapDir = hit.dir;

				if (hit.dir == DIR_POS_Y)
				{
					selectedItem->dir[0] = 0;
					selectedItem->dir[1] = 0;
					selectedItem->dir[2] = 0;
				}
				else if (hit.dir == DIR_NEG_Y)
				{
					selectedItem->dir[0] = 180;
					selectedItem->dir[1] = 0;
					selectedItem->dir[2] = 0;
				}
				else if (hit.dir == DIR_POS_X)
				{
					selectedItem->dir[0] = 0;
					selectedItem->dir[1] = 0;
					selectedItem->dir[2] = -90;
				}
				else if (hit.dir == DIR_NEG_X)
				{
					selectedItem->dir[0] = 0;
					selectedItem->dir[1] = 0;
					selectedItem->dir[2] = 90;
				}
				else if (hit.dir == DIR_POS_Z)
				{
					selectedItem->dir[0] = 90;
					selectedItem->dir[1] = 0;
					selectedItem->dir[2] = 0;
				}
				else if (hit.dir == DIR_NEG_Z)
				{
					selectedItem->dir[0] = -90;
					selectedItem->dir[1] = 0;
					selectedItem->dir[2] = 0;
				}
				updateItemTransformRot(selectedItem);
			}
			else
				updateItemTransform(selectedItem);
		}
	}
	if (mouseMode == MODE_ROTATE)
	{
		float rotStep = mx - mouseX;
		rotStep /= 80.0f;
		rotStep = floorf(rotStep);
		rotStep *= 90.0f;
		rotStep = glm_rad(rotStep);

		vec3 axis = {0, 1, 0};

		vec4 newQuat;

		glm_quatv(newQuat, rotStep, axis);

		vec4 newItemQuat;
		glm_quat_mul(itemQuat, newQuat, newItemQuat);
		memcpy(selectedItem->quat, newItemQuat, sizeof(vec4));

		updateItemTransform(selectedItem);
	}

	if (isSelecting())
	{
		calcSelectAxis();
		updateSelection(mouseDir);
	}
}

void mouseZoomCallback(GLFWwindow* window, double x, double y)
{
	if (!viewportHovered)
		return;
	cameraPos[0] += forward[0] * y * 0.1;
	cameraPos[1] += forward[1] * y * 0.1;
	cameraPos[2] += forward[2] * y * 0.1;
}
