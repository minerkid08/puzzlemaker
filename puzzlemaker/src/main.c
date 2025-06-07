#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "renderer/renderer.h"
#include <assert.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <math.h>

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

#define DIR_POS_X 0
#define DIR_NEG_X 1
#define DIR_POS_Y 2
#define DIR_NEG_Y 3
#define DIR_POS_Z 4
#define DIR_NEG_Z 5

vec3 normals[] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
vec3 coords[] = {{0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}};

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define MAP_SIZE 20
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
mat4 rotMat;
vec3 cameraPos = {0, 0, 0};
vec3 cameraRot = {0, GLM_PI, 0};
Voxel* currentVoxel;
char currentDir;

vec4 normalCol = {1, 1, 1, 1};
vec4 selectCol = {0, 1, 0, 1};

#define col(dir) (voxel == currentVoxel && dir == currentDir ? selectCol : normalCol)

void initRenderer2()
{
	initRenderer();
	float near = 0.1f;
	float far = 100.0f;
	float fov = 45.0f;
	float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	float zoom = 1.0f;
	glm_perspective(glm_rad(fov), aspect, near, far, projMat);
	// glm_ortho(-aspect * zoom, aspect * zoom, zoom, -zoom, near, far, projMat);
	setProjMat(projMat);
}

void voxelPush()
{
	if (currentVoxel != 0)
		currentVoxel->solid = 0;
}

void voxelPull()
{
	vec4 dir = {0, 0, -1, 1};

	glm_mat4_mulv(rotMat, dir, dir);

	vec3 start = {cameraPos[0], cameraPos[1], cameraPos[2]};
	vec3 end = {cameraPos[0] + dir[0] * 10, cameraPos[1] + dir[1] * 10, cameraPos[2] + dir[2] * 10};

	char hitAir = 0;

	for (float i = 0; i < 1.0f; i += 0.1f)
	{
		vec3 out;
		glm_vec3_lerp(start, end, i, out);
		int x = floorf(out[0]);
		int y = floorf(out[1]);
		int z = floorf(out[2]);

		if (x < 0 || x >= MAP_SIZE)
			continue;
		if (y < 0 || y >= MAP_SIZE)
			continue;
		if (z < 0 || z >= MAP_SIZE)
			continue;

		Voxel* v = getVoxel(x, y, z);
		if (v->solid)
		{
			if (hitAir)
			{
				v->solid = 0;

        float maxX = 9999;

        for(int i = 0; i < 5; i++)
        {
          float d = glm_dot(coords[i], dir);
          
          if(glm_dot(normals[i], dir) == 0.0f)
            continue;
          
          float x = (d - glm_dot(normals[i], start)) / glm_dot(normals[i], dir);
          if(x < maxX)
          {
            maxX = x;
            currentDir = i;
          }
        }

				return;
			}
		}
		else
			hitAir = 1;
	}
}

void drawVoxels()
{
	vec4 dir = {0, 0, -1, 1};

	glm_mat4_mulv(rotMat, dir, dir);

	vec3 start = {cameraPos[0], cameraPos[1], cameraPos[2]};
	vec3 end = {cameraPos[0] + dir[0] * 10, cameraPos[1] + dir[1] * 10, cameraPos[2] + dir[2] * 10};

	char hitAir = 0;

  currentVoxel = 0;
	for (float i = 0; i < 1.0f; i += 0.01f)
	{
		vec3 out;
		glm_vec3_lerp(start, end, i, out);
		int x = floorf(out[0]);
		int y = floorf(out[1]);
		int z = floorf(out[2]);

		if (x < 0 || x >= MAP_SIZE)
			continue;
		if (y < 0 || y >= MAP_SIZE)
			continue;
		if (z < 0 || z >= MAP_SIZE)
			continue;

		Voxel* v = getVoxel(x, y, z);
		if (v->solid)
		{
			if (hitAir)
			{
				currentVoxel = v;

        float maxX = 9999;

        for(int i = 0; i < 5; i++)
        {
          float d = glm_dot(coords[i], dir);
          
          if(glm_dot(normals[i], dir) == 0.0f)
            continue;
          
          float x = (d - glm_dot(normals[i], start)) / glm_dot(normals[i], dir);
          if(x < maxX)
          {
            maxX = x;
            currentDir = i;
          }
        }

				break;
			}
		}
		else
			hitAir = 1;
	}

	mat4 camMat;
	glm_mat4_identity(camMat);
	glm_translate(camMat, cameraPos);
	glm_rotate_z(camMat, cameraRot[2], camMat);
	glm_rotate_y(camMat, cameraRot[1], camMat);
	glm_rotate_x(camMat, cameraRot[0], camMat);

	glm_mat4_inv_fast(camMat, camMat);

	setCamMat(camMat);

	glm_mat4_identity(rotMat);
	glm_rotate_z(rotMat, cameraRot[2], rotMat);
	glm_rotate_y(rotMat, cameraRot[1], rotMat);
	glm_rotate_x(rotMat, cameraRot[0], rotMat);

	vec3 verts[4] = {{0.5, 0.5, -2}, {0.5, -0.5, -2}, {-0.5, 0.5, -2}, {-0.5, -0.5, -2}};
	vec3 verts2[4] = {{0.5, 0.5, -2}, {0.5, 0.5, -1}, {0.5, -0.5, -2}, {0.5, -0.5, -1}};
	vec3 verts3[4] = {{-0.5, 0.5, -2}, {-0.5, 0.5, -1}, {-0.5, -0.5, -2}, {-0.5, -0.5, -1}};

	for (int z = 0; z < MAP_SIZE; z++)
	{
		for (int y = 0; y < MAP_SIZE; y++)
		{
			for (int x = 0; x < MAP_SIZE; x++)
			{
				Voxel* voxel = getVoxel(x, y, z);
				if (voxel->solid)
				{
					if (z + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y, z + 1);
						if (!v2->solid)
						{
							vec3 verts[4] = {
								{x, y, z + 1}, {x + 1, y, z + 1}, {x, y + 1, z + 1}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, col(DIR_NEG_Z));
						}
					}

					if (z - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y, z - 1);
						if (!v2->solid)
						{
							vec3 verts[4] = {{x, y, z}, {x, y + 1, z}, {x + 1, y, z}, {x + 1, y + 1, z}};
							drawVerts(verts, col(DIR_POS_Z));
						}
					}

					if (x + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x + 1, y, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {
								{x + 1, y, z}, {x + 1, y + 1, z}, {x + 1, y, z + 1}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, col(DIR_NEG_X));
						}
					}

					if (x - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x - 1, y, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {{x, y, z}, {x, y, z + 1}, {x, y + 1, z}, {x, y + 1, z + 1}};
							drawVerts(verts, col(DIR_POS_X));
						}
					}

					if (y + 1 < MAP_SIZE)
					{
						Voxel* v2 = getVoxel(x, y + 1, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {
								{x, y + 1, z}, {x, y + 1, z + 1}, {x + 1, y + 1, z}, {x + 1, y + 1, z + 1}};
							drawVerts(verts, col(DIR_NEG_Y));
						}
					}

					if (y - 1 >= 0)
					{
						Voxel* v2 = getVoxel(x, y - 1, z);
						if (!v2->solid)
						{
							vec3 verts[4] = {{x, y, z}, {x + 1, y, z}, {x, y, z + 1}, {x + 1, y, z + 1}};
							drawVerts(verts, col(DIR_POS_Y));
						}
					}
				}
			}
		}
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

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
	getVoxel(2, 3, 2)->solid = 0;
	getVoxel(2, 2, 3)->solid = 0;
	getVoxel(2, 3, 3)->solid = 0;

	getVoxel(3, 2, 2)->solid = 0;
	getVoxel(3, 3, 2)->solid = 0;
	getVoxel(3, 2, 3)->solid = 0;
	getVoxel(3, 3, 3)->solid = 0;

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "puzzlemaker", 0, 0);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	initRenderer2();

	glfwSetKeyCallback(window, keyCallback);

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	// glClearColor(0, 0, 0, 1);

	double now = glfwGetTime();
	double dt;
	double lastTime = now;
	while (!glfwWindowShouldClose(window))
	{
		now = glfwGetTime();
		dt = now - lastTime;
		lastTime = now;

		if (glfwGetKey(window, GLFW_KEY_W))
		{
			cameraPos[2] -= dt * cos(cameraRot[1]);
			cameraPos[0] -= dt * sin(cameraRot[1]);
		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			cameraPos[2] += dt * cos(cameraRot[1]);
			cameraPos[0] += dt * sin(cameraRot[1]);
		}
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			cameraPos[0] -= dt * cos(cameraRot[1]);
			cameraPos[2] += dt * sin(cameraRot[1]);
		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			cameraPos[0] += dt * cos(cameraRot[1]);
			cameraPos[2] -= dt * sin(cameraRot[1]);
		}
		if (glfwGetKey(window, GLFW_KEY_Q))
			cameraPos[1] += dt;
		if (glfwGetKey(window, GLFW_KEY_E))
			cameraPos[1] -= dt;

		if (glfwGetKey(window, GLFW_KEY_LEFT))
			cameraRot[1] -= dt;
		if (glfwGetKey(window, GLFW_KEY_RIGHT))
			cameraRot[1] += dt;
		if (glfwGetKey(window, GLFW_KEY_UP))
			cameraRot[0] -= dt;
		if (glfwGetKey(window, GLFW_KEY_DOWN))
			cameraRot[0] += dt;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawVoxels();
		endFrame();

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_1)
		{
			voxelPush();
		}
	}
}
