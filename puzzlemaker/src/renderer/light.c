#include "light.h"
#include "antline/antline.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "item/item.h"
#include "renderer/renderer.h"
#include "voxel/voxel.h"
#include <glad/glad.h>
#include <string.h>

#define SHADOW_SIZE 1024
#define ATLAS_SIZE 4

static char shadowAllocation[ATLAS_SIZE * ATLAS_SIZE];

static unsigned int buffer;
static int bindingPoint = 1;
static int blockIndex;
static int meshBlockIndex;
char shadow = 0;

unsigned int shadowMap;
unsigned int shadowMapPreview;
static unsigned int shadowBuffer;

extern unsigned int shader;
extern unsigned int meshShader;
extern mat4 shadowCamMat;
extern mat4 shadowProjMat;

#define LIGHT_COUNT 128

static Light lights[LIGHT_COUNT];

#define glErrCheck()                                                                                                   \
	{                                                                                                                  \
		int error = glGetError();                                                                                      \
		while (error)                                                                                                  \
		{                                                                                                              \
			printf("gl error: %d, %d\n", __LINE__, error);                                                             \
			error = glGetError();                                                                                      \
		}                                                                                                              \
	}

void initLights()
{
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * LIGHT_COUNT, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	blockIndex = glGetUniformBlockIndex(shader, "LightBlock");
	glUniformBlockBinding(shader, blockIndex, bindingPoint);
	meshBlockIndex = glGetUniformBlockIndex(meshShader, "LightBlock");
	glUniformBlockBinding(meshShader, meshBlockIndex, bindingPoint);

	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer);

	for (int i = 0; i < LIGHT_COUNT; i++)
		lights[i].position[0] = -1;

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	int size = ATLAS_SIZE * SHADOW_SIZE;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glGenTextures(1, &shadowMapPreview);
	glBindTexture(GL_TEXTURE_2D, shadowMapPreview);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &shadowBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMapPreview, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	shadow = 0;

	for (int i = 0; i < ATLAS_SIZE * ATLAS_SIZE; i++)
		shadowAllocation[i] = 0;
}

void updateShadows(Light* light)
{
	if (light->cutoff == 0)
		return;

	if (light->shadowMapIndex == 0)
	{
		for (int i = 0; i < ATLAS_SIZE * ATLAS_SIZE; i++)
		{
			if (shadowAllocation[i] == 0)
			{
				light->shadowMapIndex = i + 1;
				shadowAllocation[i] = 1;
				break;
			}
		}
		if (light->shadowMapIndex == 0)
			return;
	}

	int x = ((light->shadowMapIndex - 1) % ATLAS_SIZE) * SHADOW_SIZE;
	int y = ((light->shadowMapIndex - 1) / ATLAS_SIZE) * SHADOW_SIZE;

	glViewport(x, y, SHADOW_SIZE, SHADOW_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glDepthMask(GL_TRUE);
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, SHADOW_SIZE, SHADOW_SIZE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	float fov = acos(light->cutoff) * 2;
	glm_mat4_identity(shadowProjMat);
	glm_perspective(fov, 1, 0.1, 50, shadowProjMat);

	vec3 angles;
	mat4 m;
	vec4 q;

	float xyDist = sqrtf(light->dirX * light->dirX + light->dirZ * light->dirZ);
	angles[0] = -atan2f(light->dirY, xyDist);
	if (xyDist > 0.001f)
		angles[1] = (atan2f(light->dirX, light->dirZ));
	else
	{
		vec3 left;
		float f2 = modff(fabsf(light->dirZ + 0.5f), &left[0]);
		left[0] = -light->dirY;
		left[1] = light->dirX - f2 * light->dirZ;
		angles[1] = atan2f(-left[0], left[1]);
	}
	angles[0] *= -1;
	angles[1] += GLM_PI;
	angles[2] = 0;

	glm_mat4_identity(m);
	glm_translate(m, light->position);
	glm_euler_yxz_quat(angles, q);
	glm_quat_mat4(q, shadowCamMat);
	glm_mat4_mul(m, shadowCamMat, m);
	glm_mat4_inv(m, shadowCamMat);
	glm_mat4_mul(shadowProjMat, shadowCamMat, m);
	glm_mat4_copy(m, light->transform);

	shadow = 1;
	setRenderPass(0);
	drawVoxels();
	renderAntlinesShadow();
	drawItemsShadow();
	endFrame();
	shadow = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
}

Light* addLight()
{
	for (int i = 0; i < LIGHT_COUNT; i++)
	{
		if (lights[i].position[0] == -1)
		{
			Light* l = &lights[i];
			memset(l, 0, sizeof(Light));
			return l;
		}
	}

	return 0;
}

void removeLight(Light* light)
{
	light->position[0] = -1;
	if (light->shadowMapIndex)
		shadowAllocation[light->shadowMapIndex - 1] = 0;
}

void updateLightBuffer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * LIGHT_COUNT, lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glErrCheck();
}
