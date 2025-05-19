#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "renderer.h"
#include <assert.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>

typedef struct
{
	char solid;
	char xPortal;
	char x2Portal;
	char yPortal;
	char y2Portal;
	char zPortal;
	char z2Portal;
} Voxel;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAP_SIZE 5
#define VOXEL_COUNT (MAP_SIZE * MAP_SIZE * MAP_SIZE)

Voxel voxels[MAP_SIZE * MAP_SIZE * MAP_SIZE];

Voxel* getVoxel(int x, int y, int z)
{
	assert(x >= 0 && x < MAP_SIZE);
	assert(y >= 0 && y < MAP_SIZE);
	assert(z >= 0 && z < MAP_SIZE);
	return &voxels[x + (y * MAP_SIZE) + (z * MAP_SIZE * MAP_SIZE)];
}

mat4 projMat;
vec3 cameraPos = {0, 0, 0};
vec3 cameraRot = {0, 0, 0};

void initRenderer2()
{
	initRenderer();
	float near = 0.01f;
	float far = 20.0f;
	float fov = 180.0f;
	float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
  float zoom = 1.0f;
  glm_ortho(-aspect * zoom, aspect * zoom, zoom, -zoom, near, far, projMat);
  //glm_perspective_default((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, projMat);
	//glm_perspective(glm_rad(fov), aspect, near, far, projMat);
  setMat(projMat);
}

void drawVoxels()
{
	//vec3 verts[4] = {{0.3, 0.3, 1}, {0.3, 0.2, 1}, {0.2, 0.3, 1}, {0.2, 0.2, 1}};
	//vec3 verts2[4] = {{0.2, 0.2, 1}, {0.2, 0.2, 5}, {0.2, 0.3, 1}, {0.2, 0.3, 5}};
	vec3 verts[4] = {{0, 0, 1}, {0, 0.1, 2}, {0.1, 0, 2}, {0.1, 0.1, 1}};

	drawVerts(verts);
	//drawVerts(verts2);
}

int main()
{
	for (int i = 0; i < VOXEL_COUNT; i++)
	{
		voxels[i].solid = 1;
		voxels[i].xPortal = 0;
		voxels[i].x2Portal = 0;
		voxels[i].yPortal = 0;
		voxels[i].y2Portal = 0;
		voxels[i].zPortal = 0;
		voxels[i].z2Portal = 0;
	}

	getVoxel(2, 2, 2)->solid = 0;

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "source -1", 0, 0);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	initRenderer2();

	glClearColor(0, 0, 0, 0);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		drawVoxels();

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
}
