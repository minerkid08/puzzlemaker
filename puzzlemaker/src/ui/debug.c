#include "cglm/mat4.h"
#include "cjson.h"
#include "dynList.h"
#include "item/entityItem.h"
#include "item/panel.h"
#include "jsonUtils.h"
#include "renderer/light.h"
#include "renderer/renderer.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "camera.h"
#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

static bool open = 0;

extern GLFWwindow* window;
extern float fps;
extern float fps2;

void copyEntityItem()
{
	ItemDefinition* def = selection.item->def;
	EntityItemDef* ent = def->data;

	cJSON* json = cJSON_CreateObject();

	switch (def->flags & ITEM_FLAGS_SNAP_MODE)
	{
	case SNAP_CENTER:
		cJSON_AddStringToObject(json, "snapMode", "center");
		break;
	case SNAP_CORNER:
		cJSON_AddStringToObject(json, "snapMode", "corner");
		break;
	case SNAP_MINI_CENTER:
		cJSON_AddStringToObject(json, "snapMode", "mini-center");
		break;
	case SNAP_MINI_CORNER:
		cJSON_AddStringToObject(json, "snapMode", "mini-corner");
		break;
	}

	jsonSetVec3(json, "bound1", ent->bound1);
	jsonSetVec3(json, "bound2", ent->bound2);

	cJSON* transform = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "transform", transform);
	jsonSetVec3(transform, "position", ent->positionOffset);
	jsonSetVec3(transform, "rotation", ent->rotationOffset);

	transform = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "editorTransform", transform);
	jsonSetVec3(transform, "position", ent->editorPos);
	jsonSetVec3(transform, "rotation", ent->editorRot);

	char* data = cJSON_Print(json);
	glfwSetClipboardString(window, data);
	cJSON_Delete(json);
	free(data);
}

void copyPropTransforms(Prop* prop)
{
	cJSON* json = cJSON_CreateObject();

	cJSON* transform = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "transform", transform);
	jsonSetVec3(transform, "position", prop->pos);
	jsonSetVec3(transform, "rotation", prop->rot);

	transform = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "editorTransform", transform);
	jsonSetVec3(transform, "position", prop->editorPos);
	jsonSetVec3(transform, "rotation", prop->editorRot);

	char* data = cJSON_Print(json);
	glfwSetClipboardString(window, data);
	cJSON_Delete(json);
	free(data);
}

void drawTransform(vec3 pos, vec3 rot, mat4 transform)
{
	char update = 0;
	update |= igDragFloat3("position", pos, 0.25, -9999, 9999, "%.3f", 0);
	update |= igDragFloat3("rotation", rot, 0.25, -9999, 9999, "%.3f", 0);
	if (update && transform)
	{
		mat4 trans;
		vec3 rot2;
		glm_mat4_identity(trans);

		mat4 rotMat;
		rot2[0] = glm_rad(rot[0]);
		rot2[1] = glm_rad(rot[1]);
		rot2[2] = glm_rad(rot[2]);
		glm_translate(trans, pos);
		glm_euler_yxz(rot2, rotMat);
		glm_mat4_mul(trans, rotMat, trans);
		memcpy(transform, trans, sizeof(mat4));
	}
}

void drawLight(Light* light)
{
	int type = light->cutoff > 0;
	if (igCombo_Str("Type", &type, "Point\0Spot\0", 2))
	{
		if (type == 0)
			light->cutoff = 0;
		else
			light->cutoff = 45;
	}
	igDragFloat3("Position", light->position, 0.1, -9999, 9999, "%.2f", 0);
	if (light->cutoff > 0)
	{
		igDragFloat("pitch", &light->dirX, 15, 0, 1, "%.2f", 0);
		igDragFloat("yaw", &light->dirY, 15, 0, 1, "%.2f", 0);
		igDragFloat("cutoff", &light->cutoff, 0.5, 0, 1, "%.2f", 0);
	}
	igColorEdit3("ambient", light->ambient, 0);
	igColorEdit3("diffuse", light->diffuse, 0);
	igColorEdit3("specular", light->specular, 0);
	igDragFloat("constant", &light->constant, 0.005, 0, 1, "%.4f", 0);
	igDragFloat("linear", &light->linear, 0.005, 0, 1, "%.4f", 0);
	igDragFloat("quadratic", &light->quadratic, 0.005, 0, 1, "%.4f", 0);
}

void openItemDebug()
{
	open = !open;
}

void itemDebugRender()
{
	if (open == 0)
		return;

	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;

	int treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;

	igBegin("Item Debug", &open, 0);

	igText("fps:  %.2f", fps);
	igText("fps2: %.2f", fps2);

	if (igTreeNodeEx_Str("Camera", treeNodeFlags))
	{
		igDragFloat3("Position", cameraPos, 0.1, -9999, 9999, "%.2f", 0);
		if (igDragFloat3("Rotation", cameraRot, 0.01, -9999, 9999, "%.2f", 0))
			cameraRot[2] = 0;
		igTreePop();
	}

	if (selection.type != SELECTION_ITEM)
	{
		igText("no item selected");
		igEnd();
		return;
	}

	if (igTreeNodeEx_Str("Item", treeNodeFlags))
	{
		ItemDefinition* def = selection.item->def;
		int itemType = selection.item->def->type;

		int snapMode = def->flags & ITEM_FLAGS_SNAP_MODE;
		if (igCombo_Str("Snap Mode", &snapMode, "Corner\0Center\0Mini Corner\0Mini Center\0", 4))
		{
			def->flags &= ~ITEM_FLAGS_SNAP_MODE;
			def->flags |= snapMode;
		}

		if (itemType == ITEM_TYPE_ENTITY)
		{
			EntityItemDef* data = def->data;
			igSeparatorText("Entity Item Options");
			igDragFloat3("Bound 1", data->bound1, 0.25, -9999, 9999, "%.3f", 0);
			igDragFloat3("Bound 2", data->bound2, 0.25, -9999, 9999, "%.3f", 0);
			if (igTreeNode_Str("Editor Transform"))
			{
				drawTransform(data->editorPos, data->editorRot, data->editorTransform);
				igTreePop();
			}

			if (igTreeNode_Str("Export Transform"))
			{
				drawTransform(data->positionOffset, data->rotationOffset, 0);
				igTreePop();
			}

			if (igButton("Copy To Clipboard", zero))
				copyEntityItem();
		}

		if (itemType == ITEM_TYPE_PANEL)
		{
			PanelDefData* data = def->data;
			igSeparatorText("Panel Item Options");
			for (int i = 0; i < dynList_size(data->props); i++)
			{
				Prop* prop = &data->props[i];
				const char* name = 0;
				switch (prop->id)
				{
				case PANEL_ITEM_ID_BOTTOM_LEFT:
					name = "Bottom Left Prop";
					break;
				case PANEL_ITEM_ID_BOTTOM:
					name = "Bottom Prop";
					break;
				case PANEL_ITEM_ID_BOTTOM_RIGHT:
					name = "Bottom Right Prop";
					break;
				case PANEL_ITEM_ID_LEFT:
					name = "Left Prop";
					break;
				case PANEL_ITEM_ID_RIGHT:
					name = "Right Prop";
					break;
				case PANEL_ITEM_ID_TOP_LEFT:
					name = "Top Left Prop";
					break;
				case PANEL_ITEM_ID_TOP:
					name = "Top Prop";
					break;
				case PANEL_ITEM_ID_TOP_RIGHT:
					name = "Top Right Prop";
					break;
				}
				if (igTreeNodeEx_Str(name, ImGuiTreeNodeFlags_DrawLinesFull))
				{
					if (igTreeNode_Str("Editor transform"))
					{
						drawTransform(prop->editorPos, prop->editorRot, prop->editorTransform);
						igTreePop();
					}
					if (igTreeNode_Str("Export transform"))
					{
						drawTransform(prop->pos, prop->rot, prop->transform);
						igTreePop();
					}
					if (igButton("Copy To Clipboard", zero))
						copyPropTransforms(prop);
					igTreePop();
				}
			}
		}
		if (igTreeNodeEx_Str("Lights", treeNodeFlags))
		{
			int len = dynList_size(selection.item->def->lights);
			for (int i = 0; i < len; i++)
			{
				char buf[4];
				snprintf(buf, 4, "%d", i);
				if (igTreeNode_Str(buf))
				{
					drawLight(&selection.item->def->lights[i]);
					igTreePop();
				}
			}

			if (igButton("Apply", zero))
			{
				Item* items = getItemList();
				int l = dynList_size(items);
				for (int i = 0; i < l; i++)
				{
					Item* item = &items[i];
					if (!isItemValid(item))
						continue;
					if (item->def != selection.item->def)
						continue;
					updateLights(item);
					updateItemTransform(item);
				}
			}

			igTreePop();
		}
		igTreePop();
	}

	igEnd();
}
