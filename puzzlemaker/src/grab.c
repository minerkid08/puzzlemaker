#include "antline/antline.h"
#include "item/item.h"
#include "raycast.h"
#include "selection.h"
#include "utils.h"
#include <string.h>

int mousex = 0;
vec4 itemQuat;

void startGrab()
{
}

void updateGrab(vec3 cameraPos, vec3 mouseDir)
{
	vec3 newPos;
	vec3 newRot;
	RaycastHit hit;
	if (raycast(cameraPos, mouseDir, 40, RAYCAST_VOXEL, &hit, 0))
	{
		float startX = hit.pos[0];
		float startY = hit.pos[1];
		float startZ = hit.pos[2];
		char snapMode = SNAP_MINI_CENTER;
		if (selection.type == SELECTION_ITEM)
			snapMode = selection.item->def->snapMode;
		switch (snapMode)
		{
		case SNAP_CORNER:
			newPos[0] = round(hit.pos[0]);
			newPos[1] = round(hit.pos[1]);
			newPos[2] = round(hit.pos[2]);
			break;
		case SNAP_CENTER:
			newPos[0] = round(hit.pos[0] - 0.5) + 0.5;
			newPos[1] = round(hit.pos[1] - 0.5) + 0.5;
			newPos[2] = round(hit.pos[2] - 0.5) + 0.5;
			break;
		case SNAP_MINI_CORNER:
			newPos[0] = round(hit.pos[0] * 2) / 2;
			newPos[1] = round(hit.pos[1] * 2) / 2;
			newPos[2] = round(hit.pos[2] * 2) / 2;
			break;
		case SNAP_MINI_CENTER:
			newPos[0] = round(hit.pos[0] * 2 - 0.25) / 2 + 0.25;
			newPos[1] = round(hit.pos[1] * 2 - 0.25) / 2 + 0.25;
			newPos[2] = round(hit.pos[2] * 2 - 0.25) / 2 + 0.25;
			break;
		}
		char snapDir;
		if (selection.type == SELECTION_ITEM)
			snapDir = selection.item->snapDir;
		if (selection.type == SELECTION_ANTLINE)
			snapDir = selection.antlineSeg->snapDir;
		switch (snapDir)
		{
		case DIR_POS_Z:
		case DIR_NEG_Z:
			newPos[2] = startZ;
			break;
		case DIR_POS_Y:
		case DIR_NEG_Y:
			newPos[1] = startY;
			break;
		case DIR_POS_X:
		case DIR_NEG_X:
			newPos[0] = startX;
			break;
		}

		if (snapDir != hit.dir)
		{

			if (selection.type == SELECTION_ITEM)
				selection.item->snapDir = hit.dir;
			if (selection.type == SELECTION_ANTLINE)
				selection.antlineSeg->snapDir = hit.dir;

			if (hit.dir == DIR_POS_Y)
			{
				newRot[0] = 0;
				newRot[1] = 0;
				newRot[2] = 0;
			}
			else if (hit.dir == DIR_NEG_Y)
			{
				newRot[0] = 180;
				newRot[1] = 0;
				newRot[2] = 0;
			}
			else if (hit.dir == DIR_POS_X)
			{
				newRot[0] = 0;
				newRot[1] = 180;
				newRot[2] = 90;
			}
			else if (hit.dir == DIR_NEG_X)
			{
				newRot[0] = 0;
				newRot[1] = 0;
				newRot[2] = 90;
			}
			else if (hit.dir == DIR_POS_Z)
			{
				newRot[0] = 0;
				newRot[1] = 90;
				newRot[2] = 90;
			}
			else if (hit.dir == DIR_NEG_Z)
			{
				newRot[0] = 0;
				newRot[1] = -90;
				newRot[2] = 90;
			}
			if (selection.type == SELECTION_ITEM)
			{
				memcpy(selection.item->pos, newPos, sizeof(vec3));
				memcpy(selection.item->dir, newRot, sizeof(vec3));
				updateItemTransformRot(selection.item);
			}
			if (selection.type == SELECTION_ANTLINE)
			{
				memcpy(selection.antlineSeg->pos, newPos, sizeof(vec3));
				memcpy(selection.antlineSeg->rot, newRot, sizeof(vec3));
				antlineUpdateTransformRot(selection.antlineSeg);
			}
		}
		else
		{
			if (selection.type == SELECTION_ITEM)
			{
				memcpy(selection.item->pos, newPos, sizeof(vec3));
				updateItemTransformRot(selection.item);
			}
			if (selection.type == SELECTION_ANTLINE)
			{
				memcpy(selection.antlineSeg->pos, newPos, sizeof(vec3));
				antlineUpdateTransformRot(selection.antlineSeg);
			}
		}
	}
}

void startRotate(int mouseX)
{
	mousex = mouseX;
	if (selection.type == SELECTION_ITEM)
		memcpy(itemQuat, selection.item->quat, sizeof(vec4));
	if (selection.type == SELECTION_ANTLINE)
		memcpy(itemQuat, selection.antlineSeg->quat, sizeof(vec4));
}

void updateRotate(int mouseX)
{
	float rotStep = mouseX - mousex;
	rotStep /= 80.0f;
	rotStep = floorf(rotStep);
	rotStep *= 90.0f;
	rotStep = glm_rad(rotStep);

	vec3 axis = {0, 1, 0};

	vec4 newQuat;

	glm_quatv(newQuat, rotStep, axis);

	vec4 newItemQuat;
	glm_quat_mul(itemQuat, newQuat, newItemQuat);
	if (selection.type == SELECTION_ITEM)
	{
		memcpy(selection.item->quat, newItemQuat, sizeof(vec4));
		updateItemTransform(selection.item);
	}
	if (selection.type == SELECTION_ANTLINE)
	{
		memcpy(selection.antlineSeg->quat, newItemQuat, sizeof(vec4));
		antlineUpdateTransform(selection.antlineSeg);
	}
}

void rotateSelection(float amount)
{
	vec4 itemQuat;
	if (selection.type == SELECTION_ITEM)
		memcpy(itemQuat, selection.item->quat, sizeof(vec4));
	if (selection.type == SELECTION_ANTLINE)
		memcpy(itemQuat, selection.antlineSeg->quat, sizeof(vec4));
	vec4 quat2;
	vec3 axis = {0, 1, 0};

	glm_quatv(quat2, glm_rad(amount), axis);

	vec4 newItemQuat;
	glm_quat_mul(itemQuat, quat2, newItemQuat);
	if (selection.type == SELECTION_ITEM)
	{
		memcpy(selection.item->quat, newItemQuat, sizeof(vec4));
		updateItemTransform(selection.item);
	}
	if (selection.type == SELECTION_ANTLINE)
	{
		memcpy(selection.antlineSeg->quat, newItemQuat, sizeof(vec4));
		antlineUpdateTransform(selection.antlineSeg);
	}
}
