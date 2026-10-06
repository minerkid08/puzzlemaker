#include "antline/antline.h"
#include "assetManager.h"
#include "grab.h"
#include "mapsettings.h"
#include "renderer/framebuffer.h"
#include "settings.h"
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

extern char queueCompile;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double x, double y);
void mouseZoomCallback(GLFWwindow* window, double x, double y);

extern float uiScale;
static char snapRotation;

int main()
{
	loadMapSettingsPresets();
	loadEditorSettings();
	makeDir("maps");

	startCompileThread();
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "puzzlemaker", 0, 0);
	glfwMakeContextCurrent(window);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	glfwGetMonitorContentScale(monitor, &uiScale, 0);
	uiScale *= 1.4;

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	initVoxels();
	loadAntlineConfig();

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
	while (!glfwWindowShouldClose(window) && 0)
	{
		framebufferBind(&framebuffer);

		now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		float moveForward = glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S);
		float moveRight = glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A);

		float lookUp = glfwGetKey(window, GLFW_KEY_UP) - glfwGetKey(window, GLFW_KEY_DOWN);
		float lookRight = glfwGetKey(window, GLFW_KEY_RIGHT) - glfwGetKey(window, GLFW_KEY_LEFT);

		snapRotation = !glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);

		float moveSpeed = editorSettings.moveSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
			moveSpeed = editorSettings.boostSpeed;

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

		cameraPos[0] += forward[0] * moveForward * dt * moveSpeed;
		cameraPos[1] += forward[1] * moveForward * dt * moveSpeed;
		cameraPos[2] += forward[2] * moveForward * dt * moveSpeed;

		cameraPos[0] += right[0] * moveRight * dt * moveSpeed;
		cameraPos[1] += right[1] * moveRight * dt * moveSpeed;
		cameraPos[2] += right[2] * moveRight * dt * moveSpeed;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawVoxels(cameraPos, cameraRot);
		endFrame();

		renderAntlines();
		drawItems();
		framebufferUnbind(&framebuffer);

		uiNewFrame();

		uiMenuBar();
		uiViewport(&framebuffer);
		fileBrowserRender();

		itemListRender();
		itemDebugRender();
		renderEditorPanel();

		uiEndFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	saveEditorSettings();
	rendererCleanup();
	debugCleanup();
	assetManagerCleanup();

	glfwTerminate();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!viewportHovered)
		return;

	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_F9)
			queueCompile = 1;
		if (key == GLFW_KEY_2)
			voxelPush();
		if (key == GLFW_KEY_3)
			voxelPull();
		if (key == GLFW_KEY_R)
			voxelTogglePortal();
		if (key == GLFW_KEY_Z)
			voxelToggleSize();
		if (key == GLFW_KEY_G)
		{
			if (selection.type == SELECTION_VOXEL)
				return;
			mouseMode = MODE_GRAB;
			startGrab();
		}
		if (key == GLFW_KEY_F)
		{
			if (selection.type == SELECTION_VOXEL)
				return;
			mouseMode = MODE_ROTATE;
			startRotate(mx);
		}
		if (key == GLFW_KEY_Q)
		{
			if (selection.type == SELECTION_VOXEL)
				return;
			rotateSelection(90);
		}
		if (key == GLFW_KEY_E)
		{
			if (selection.type == SELECTION_VOXEL)
				return;
			rotateSelection(90);
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
		calcSelectAxis();
		updateGrab(cameraPos, mouseDir);
	}
	if (mouseMode == MODE_ROTATE)
		updateRotate(mx);

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
	cameraPos[0] += forward[0] * y * 0.4;
	cameraPos[1] += forward[1] * y * 0.4;
	cameraPos[2] += forward[2] * y * 0.4;
}
