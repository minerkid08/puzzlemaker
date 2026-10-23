#include "antline/antline.h"
#include "assetManager.h"
#include "dynList.h"
#include "grab.h"
#include "item/item.h"
#include "mapsettings.h"
#include "packages/package.h"
#include "renderer/framebuffer.h"
#include "save.h"
#include "settings.h"
#include "ui/fileBrowser.h"
#include "utils.h"
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>

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
unsigned int mouseOffsetX = 0;
unsigned int mouseOffsetY = 0;

Item** pickPtr = 0;

extern char queueCompile;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double x, double y);
void mouseZoomCallback(GLFWwindow* window, double x, double y);

extern float uiScale;
static char snapRotation;

const char* commandLine = 0;
GLFWwindow* window;
char running = 0;

int main(int argc, const char** argv)
{
	commandLine = argv[0];
	makeDir("maps");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(1920, 1080, "puzzlemaker", 0, 0);
	glfwMakeContextCurrent(window);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	glfwGetMonitorContentScale(monitor, &uiScale, 0);
	uiScale *= 1.4;

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseZoomCallback);

	glfwSwapInterval(1);

	initRenderer();

	initUi(window);

	startCompileThread();

	loadPackageList();

	loadEditorSettings();
	initVoxels();

	aspect = (float)width / (float)height;
	initCamera();

	initItemPanel();

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	FrameBuffer framebuffer;
	framebuffer.width = 1920;
	framebuffer.height = 1080;
	framebufferInit(&framebuffer);

	double now = glfwGetTime();
	double dt;
	double lastTime = now;
	running = 1;
	while (!glfwWindowShouldClose(window))
	{
		framebufferBind(&framebuffer);

		now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		float moveForward = 0.0f;
		float moveRight = 0.0f;
		float lookUp = 0.0f;
		float lookRight = 0.0f;

		if (viewportHovered)
		{
			moveForward = glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S);
			moveRight = glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A);

			lookUp = glfwGetKey(window, GLFW_KEY_UP) - glfwGetKey(window, GLFW_KEY_DOWN);
			lookRight = glfwGetKey(window, GLFW_KEY_RIGHT) - glfwGetKey(window, GLFW_KEY_LEFT);

			lookUp *= editorSettings.arrowSens;
			lookRight *= editorSettings.arrowSens;
		}

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

		cameraMoveForward(moveForward * dt * moveSpeed);
		cameraMoveRight(moveRight * dt * moveSpeed);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawVoxels();
		endFrame();

		renderAntlines();
		drawItems();
		renderAntlineHover();
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

extern int queueItemAdd;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_F9)
		{
			if (mapSettings.name[0] == 0)
				fileBrowserOpen(MODE_SAVE | QUEUE_COMPILE);
			else
				queueCompile = 1;
			return;
		}
		if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
		{
			fileBrowserSave();
			return;
		}
		if (!viewportHovered)
			return;
		if (mods & GLFW_MOD_CONTROL)
		{
			if (key == GLFW_KEY_C)
			{
				if (selection.type != SELECTION_ITEM)
					return;
				copyItem(selection.item);
				return;
			}
			if (key == GLFW_KEY_V)
			{
				vec3 offset;
				vec3 pos;
				ivec3 ipos;

				glm_vec3_scale(forward, 5, offset);

				pos[0] = offset[0] + cameraPos[0];
				pos[1] = offset[1] + cameraPos[1];
				pos[2] = offset[2] + cameraPos[2];

				selection.item = pasteItem();
				if (selection.item)
				{
					selection.type = SELECTION_ITEM;

					selection.item->pos[0] = pos[0];
					selection.item->pos[1] = pos[1];
					selection.item->pos[2] = pos[2];
					updateItemTransform(selection.item);
					mouseMode = MODE_GRAB;
				}
				return;
			}
		}
	}

	if (!viewportHovered)
		return;

	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_2)
		{
			if (selection.type == SELECTION_VOXEL)
				voxelPush();
			if (selection.type == SELECTION_ANTLINE)
				selection.antlineSeg->len++;
		}
		if (key == GLFW_KEY_3)
		{
			if (selection.type == SELECTION_VOXEL)
				voxelPull();
			if (selection.type == SELECTION_ANTLINE)
			{
				if (selection.antlineSeg->len > 1)
					selection.antlineSeg->len--;
			}
		}
		if (key == GLFW_KEY_R)
		{
			if (selection.type == SELECTION_VOXEL)
				voxelTogglePortal();
		}
		if (key == GLFW_KEY_Z)
		{
			if (selection.type == SELECTION_VOXEL)
				voxelToggleSize();
		}
		if (key == GLFW_KEY_G)
		{
			if (selection.type == SELECTION_VOXEL || selection.type == SELECTION_NONE)
				return;
			mouseMode = MODE_GRAB;
			startGrab();
		}
		if (key == GLFW_KEY_F)
		{
			if (selection.type == SELECTION_VOXEL || selection.type == SELECTION_NONE)
				return;
			mouseMode = MODE_ROTATE;
			startRotate(mx);
		}
		if (key == GLFW_KEY_Q)
		{
			if (selection.type == SELECTION_VOXEL || selection.type == SELECTION_NONE)
				return;
			rotateSelection(90);
		}
		if (key == GLFW_KEY_E)
		{
			if (selection.type == SELECTION_VOXEL || selection.type == SELECTION_NONE)
				return;
			rotateSelection(-90);
		}
		if (key == GLFW_KEY_H)
		{
			if (mods & GLFW_MOD_ALT)
			{
				if (selection.type == SELECTION_ITEM)
					selection.item->hidden = 0;
				else
				{
					Item* itemList = getItemList();
					for (int i = 0; i < dynList_size(itemList); i++)
					{
						Item* item = &itemList[i];
						if (isItemValid(item))
							item->hidden = 0;
					}
				}
			}
			else
			{
				if (selection.type == SELECTION_ITEM)
					selection.item->hidden = 1;
				else
				{
					Item* itemList = getItemList();
					for (int i = 0; i < dynList_size(itemList); i++)
					{
						Item* item = &itemList[i];
						if (isItemValid(item))
							item->hidden = 1;
					}
				}
			}
			if (selection.type == SELECTION_VOXEL)
				voxelToggleSize();
		}
		if (key == GLFW_KEY_C)
			queueItemAdd = 1;
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
			if (editorSettings.lockMouseOnPan)
			{
				tx = width / 2.0f;
				ty = height / 2.0f;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				int viewportCenterX = mouseOffsetX + width / 2;
				int viewportCenterY = mouseOffsetY + height / 2;
				glfwSetCursorPos(window, viewportCenterX, viewportCenterY);
			}
		}

		if (button == GLFW_MOUSE_BUTTON_3)
		{
			mouseMode = MODE_PAN;
			memcpy(tempPos, cameraPos, sizeof(float) * 3);
			if (editorSettings.lockMouseOnPan)
			{
				tx = width / 2.0f;
				ty = height / 2.0f;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				int viewportCenterX = mouseOffsetX + width / 2;
				int viewportCenterY = mouseOffsetY + height / 2;
				glfwSetCursorPos(window, viewportCenterX, viewportCenterY);
			}
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
		if (editorSettings.lockMouseOnPan)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

	double dx = 0;
	double dy = 0;
	if (editorSettings.lockMouseOnPan)
	{
		dx = width / 2.0f - x2;
		dy = height / 2.0f - y2;
		if (mouseMode == MODE_ORBIT)
		{
			cameraRot[1] += (dx * editorSettings.orbitSens / 100.0f);
			cameraRot[0] += (dy * editorSettings.orbitSens / 100.0f);

			int viewportCenterX = mouseOffsetX + width / 2;
			int viewportCenterY = mouseOffsetY + height / 2;
			glfwSetCursorPos(window, viewportCenterX, viewportCenterY);
		}

		if (mouseMode == MODE_PAN)
		{
			cameraPos[0] += right[0] * dx * editorSettings.panSens / 100.0f;
			cameraPos[1] += right[1] * dx * editorSettings.panSens / 100.0f;
			cameraPos[2] += right[2] * dx * editorSettings.panSens / 100.0f;

			cameraPos[0] -= up[0] * dy * editorSettings.panSens / 100.0f;
			cameraPos[1] -= up[1] * dy * editorSettings.panSens / 100.0f;
			cameraPos[2] -= up[2] * dy * editorSettings.panSens / 100.0f;

			int viewportCenterX = mouseOffsetX + width / 2;
			int viewportCenterY = mouseOffsetY + height / 2;
			glfwSetCursorPos(window, viewportCenterX, viewportCenterY);
		}
	}
	else
	{
		dx = tx - x2;
		dy = ty - y2;
		if (mouseMode == MODE_ORBIT)
		{
			cameraRot[1] = tempRot[1] + (dx * editorSettings.orbitSens / 100.0f);
			cameraRot[0] = tempRot[0] + (dy * editorSettings.orbitSens / 100.0f);
		}

		if (mouseMode == MODE_PAN)
		{
			cameraPos[0] = tempPos[0] + right[0] * dx * editorSettings.panSens / 100.0f;
			cameraPos[1] = tempPos[1] + right[1] * dx * editorSettings.panSens / 100.0f;
			cameraPos[2] = tempPos[2] + right[2] * dx * editorSettings.panSens / 100.0f;

			cameraPos[0] -= up[0] * dy * editorSettings.panSens / 100.0f;
			cameraPos[1] -= up[1] * dy * editorSettings.panSens / 100.0f;
			cameraPos[2] -= up[2] * dy * editorSettings.panSens / 100.0f;
		}
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
