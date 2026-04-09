#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cimgui.h"
#include "renderer/renderer.h"
#include <assert.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string.h>

#include "voxel.h"
#include "camera.h"
#include "utils.h"
#include "voxelModification.h"
#include "item.h"
#include "ui.h"

#define MODE_NONE 0
#define MODE_ORBIT 1
#define MODE_PAN 2

double tx;
double ty;

double mx;
double my;

vec3 tempPos;
vec3 tempRot;

void findSelected()
{
	mat4 rotMat;

	glm_mat4_identity(rotMat);
	glm_rotate_z(rotMat, cameraRot[2], rotMat);
	glm_rotate_y(rotMat, cameraRot[1], rotMat);
	glm_rotate_x(rotMat, cameraRot[0], rotMat);

	vec4 dir = {0, 0, -1, 1};

	dir[1] = (fovy / 2) - (my / WINDOW_HEIGHT) * fovy;
	dir[0] = (-fovx / 2) + (mx / WINDOW_WIDTH) * fovx;

	glm_mat4_mulv(rotMat, dir, dir);

	currentVoxel = 0;
	RaycastHit hit;
	if (voxelRaycast(cameraPos, dir, 10, &hit))
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

int main()
{
  initItems();

  ItemDefinition def;
  def.name = "testItem";
  def.boundingBox[0] = 1.0f;
  def.boundingBox[1] = 1.0f;
  def.boundingBox[2] = 0.1f;

  def.faceRestrictions = FaceRestriction_Wall;
  def.snappingMode = SnappingMode_Center;

  def.material = "item.png";

  addItemDef(&def);

	initVoxels();

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "puzzlemaker", 0, 0);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  initRenderer();
  initCamera();

  initUi(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseZoomCallback);

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	double now = glfwGetTime();
	double dt;
	double lastTime = now;
	while (!glfwWindowShouldClose(window))
	{
		now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

    updateCamera();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawVoxels(cameraPos, cameraRot);
		endFrame();

    uiNewFrame();

    igBegin("demo", 0, 0);
    igShowDemoWindow(0);
    igEnd();

    uiEndFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_1)
			voxelPush();
		if (key == GLFW_KEY_2)
			voxelPull();
		if (key == GLFW_KEY_3)
			voxelTogglePortal();

    if(key == GLFW_KEY_4)
      addItem(0);
	}
}

int mouseMode = 0;

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
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
			findSelected();
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
	cameraPos[0] += forward[0] * y * 0.1;
	cameraPos[1] += forward[1] * y * 0.1;
	cameraPos[2] += forward[2] * y * 0.1;
}
