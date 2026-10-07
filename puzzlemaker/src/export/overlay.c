#include "export/overlay.h"
#include "cglm/mat4.h"
#include "cglm/quat.h"
#include "dynList.h"
#include "export/brush.h"
#include "export/entity.h"
#include "voxel/itemIntersection.h"
#include <stdio.h>
#include <string.h>

static Overlay* overlays;

static __attribute__((constructor)) void init()
{
	overlays = dynList_new(0, sizeof(Overlay));
	dynList_reserve((void**)&overlays, 32);
}

Overlay* getOverlayList()
{
	return overlays;
}

void exportStartOverlays()
{
	dynList_resize((void**)&overlays, 0);
}

Overlay* exportCreateOverlay()
{
	int len = dynList_size(overlays);
	dynList_resize((void**)&overlays, len + 1);
	Overlay* overlay = &overlays[len];

	overlay->tint[0] = 255;
	overlay->tint[1] = 255;
	overlay->tint[2] = 255;
	overlay->tint[3] = 255;

	return overlay;
}

void exportEndOverlays(FILE* file)
{
	int startId = dynList_size(getEntityList());
	int len = dynList_size(overlays);
	for (int i = 0; i < len; i++)
	{
		Overlay* overlay = &overlays[i];

		fprintf(file, "entity\n{\n");

		fprintf(file, "  \"id\" \"%d\"\n", i + startId);
		fprintf(file, "  \"classname\" \"info_overlay\"\n");
		fprintf(file, "  \"origin\" \"%f %f %f\"\n", overlay->pos[2] * 64, overlay->pos[0] * 64, overlay->pos[1] * 64);
		fprintf(file, "  \"angles\" \"0 0 0\"\n");
		fprintf(file, "  \"targetname\" \"%s\"\n", overlay->name);
		fprintf(file, "  \"BasisOrigin\" \"%f %f %f\"\n", overlay->pos[2] * 64, overlay->pos[0] * 64,
				overlay->pos[1] * 64);

		free((char*)overlay->name);

		vec3 normDir = {0, 1, 0};
		vec3 uDir = {0, 0, 1};
		vec3 vDir = {1, 0, 0};

		vec3 rot;
		memcpy(rot, overlay->rotation, sizeof(vec3));
		rot[0] = glm_rad(rot[0]);
		rot[1] = glm_rad(rot[1]);
		rot[2] = glm_rad(rot[2]);
		mat4 rotMat;
		vec4 quat;
		glm_euler_yzx_quat(rot, quat);
		glm_quat_mat4(quat, rotMat);
		glm_mat4_mulv3(rotMat, normDir, 1, normDir);
		glm_mat4_mulv3(rotMat, uDir, 1, uDir);
		glm_mat4_mulv3(rotMat, vDir, 1, vDir);
		float tmp = normDir[0];
		normDir[0] = normDir[2];
		normDir[2] = normDir[1];
		normDir[1] = tmp;
		tmp = uDir[0];
		uDir[0] = uDir[2];
		uDir[2] = uDir[1];
		uDir[1] = tmp;
		tmp = vDir[0];
		vDir[0] = vDir[2];
		vDir[2] = vDir[1];
		vDir[1] = tmp;
		fprintf(file, "  \"BasisNormal\" \"%f %f %f\"\n", normDir[0], normDir[1], normDir[2]);
		fprintf(file, "  \"BasisU\" \"%f %f %f\"\n", uDir[0], uDir[1], uDir[2]);
		fprintf(file, "  \"BasisV\" \"%f %f %f\"\n", vDir[0], vDir[1], vDir[2]);
		fprintf(file, "  \"material\" \"%s\"\n", overlay->texture);
		fprintf(file, "  \"tint\" \"%f %f %f %f\"\n", overlay->tint[0], overlay->tint[1], overlay->tint[2],
				overlay->tint[3]);
		fprintf(file, "  \"startu\" \"0\"\n");
		fprintf(file, "  \"startv\" \"%f\"\n", overlay->tile[0]);
		fprintf(file, "  \"endu\" \"%f\"\n", overlay->tile[1]);
		fprintf(file, "  \"endv\" \"0\"\n");

		if(overlay->script)
			free((char*)overlay->texture);

		float halfWidth = overlay->size[0] / 2.0f;
		float halfHeight = overlay->size[1] / 2.0f;

		fprintf(file, "  \"uv0\" \"%f %f 0\"\n", -halfWidth, -halfHeight);
		fprintf(file, "  \"uv1\" \"%f %f 0\"\n", -halfWidth, halfHeight);
		fprintf(file, "  \"uv2\" \"%f %f 0\"\n", halfWidth, halfHeight);
		fprintf(file, "  \"uv3\" \"%f %f 0\"\n", halfWidth, -halfHeight);
		Brush* brushList = getBrushArray();
		int brushLen = dynList_size(brushList);
		OBB overlayObb;
		vec3 min;
		vec3 max;
		min[0] = -overlay->size[1] / 128.0f;
		min[1] = -0.125;
		min[2] = -overlay->size[0] / 128.0f;
		max[0] = overlay->size[1] / 128.0f;
		max[1] = 0.125;
		max[2] = overlay->size[0] / 128.0f;
		min[0] += 0.01;
		min[1] += 0.01;
		min[2] += 0.01;
		max[0] -= 0.01;
		max[1] -= 0.01;
		max[2] -= 0.01;
		normDir[0] *= -1;
		normDir[1] *= -1;
		normDir[2] *= -1;
		genOBB(min, max, rotMat, overlay->pos, &overlayObb);
		char buf[256];
		buf[0] = 0;
		int bufLen = 0;
		for (int j = 0; j < brushLen; j++)
		{
			Brush* brush = &brushList[j];
			for (int k = 0; k < 6; k++)
			{
				Side* side = &brush->sides[k];
				vec3 normal;
				memcpy(normal, side->normal, sizeof(vec3));
				float d = glm_vec3_dot(normal, normDir);
				if (d > 0.9999f)
				{
					OBB obb;
					vec3 rot;
					memcpy(rot, brush->rot, sizeof(vec3));
					rot[0] = glm_rad(rot[0]);
					rot[1] = glm_rad(rot[1]);
					rot[2] = glm_rad(rot[2]);
					mat4 rotMat;
					glm_euler_yzx(rot, rotMat);
					genOBB(brush->bound1, brush->bound2, rotMat, brush->pos, &obb);
					if (getCollision(&overlayObb, &obb))
						bufLen += snprintf(buf + bufLen, 256 - bufLen, "%d ", side->id);
				}
			}
		}
		fprintf(file, "  \"sides\" \"%s\"\n", buf);
		fprintf(file, "}\n");
	}
}
