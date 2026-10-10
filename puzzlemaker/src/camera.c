#include "camera.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "renderer/renderer.h"
#include "settings.h"
#include <cglm/cglm.h>

float fovx;
float fovy;

mat4 projMat;
mat4 projMatInv;
vec4 cameraPos = {3, 3, -2, 1};
vec4 cameraRot = {0, GLM_PI, 0, 1};

vec4 forward;
vec4 up;
vec4 right;

float aspect;

void initCamera()
{
	float near = 0.1f;
	float far = 100.0f;

	fovy = glm_rad(editorSettings.fov / 2);
	fovx = 2 * atanf(tanf(fovy / 2) * aspect);

	glm_perspective(fovy, aspect, near, far, projMat);
	setProjMat(projMat);
	glm_mat4_inv(projMat, projMatInv);
}

void updateCamera()
{
	mat4 rotMat;

	glm_mat4_identity(rotMat);
	glm_rotate_z(rotMat, cameraRot[2], rotMat);
	glm_rotate_y(rotMat, cameraRot[1], rotMat);
	glm_rotate_x(rotMat, cameraRot[0], rotMat);

	vec4 forward2 = {0, 0, -1, 1};
	vec4 up2 = {0, 1, 0, 1};
	vec4 right2= {1, 0, 0, 1};

	glm_mat4_mulv(rotMat, forward2, forward);
	glm_mat4_mulv(rotMat, up2, up);
	glm_mat4_mulv(rotMat, right2, right);

	mat4 camMat;
	glm_mat4_identity(camMat);
	glm_translate(camMat, cameraPos);
	glm_rotate_z(camMat, cameraRot[2], camMat);
	glm_rotate_y(camMat, cameraRot[1], camMat);
	glm_rotate_x(camMat, cameraRot[0], camMat);

	glm_mat4_inv_fast(camMat, camMat);

	setCamMat(camMat);
}

void cameraMoveForward(float moveForward)
{
	cameraPos[0] += forward[0] * moveForward;
	cameraPos[1] += forward[1] * moveForward;
	cameraPos[2] += forward[2] * moveForward;
}

void cameraMoveRight(float moveRight)
{
	cameraPos[0] += right[0] * moveRight;
	cameraPos[1] += right[1] * moveRight;
	cameraPos[2] += right[2] * moveRight;
}
