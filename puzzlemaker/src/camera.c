#include "camera.h"
#include "cglm/cglm.h"
#include "cglm/mat4.h"
#include "renderer/renderer.h"
#include "utils.h"

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

	fovy = glm_rad(45.0f);
	fovx = 2 * atanf(tanf(fovy / 2) * aspect);

	glm_perspective(fovy, aspect, near, far, projMat);
	setProjMat(projMat);
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
	vec4 right2 = {1, 0, 0, 1};

	glm_mat4_mulv(rotMat, forward2, forward);
	glm_mat4_mulv(rotMat, up2, up);
	glm_mat4_mulv(rotMat, right2, right);
	glm_mat4_inv(projMat, projMatInv);
}
