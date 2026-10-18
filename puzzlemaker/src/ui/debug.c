#include "cglm/mat4.h"
#include "item/entityItem.h"
#include "item/panel.h"
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

static bool open = 0;

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

	igBegin("Item Debug", &open, 0);

	if (selection.type != SELECTION_ITEM)
	{
		igText("no item selected");
		igEnd();
		return;
	}

	ItemDefinition* def = selection.item->def;
	int itemType = selection.item->def->type;

	igSeparatorText("General Options");
	int snapMode = def->snapMode;
	if (igCombo_Str("snap mode", &snapMode, "corner\0center\0mini corner\0mini center\0", 4))
		def->snapMode = snapMode;
	igCheckbox("transparent", (bool*)&def->transparent);

	if (itemType == ITEM_TYPE_ENTITY)
	{
		EntityItemDef* data = def->data;
		igSeparatorText("Entity Item Options");
		igDragFloat3("bound1", data->bound1, 0.25, -9999, 9999, "%.3f", 0);
		igDragFloat3("bound2", data->bound2, 0.25, -9999, 9999, "%.3f", 0);
		if (igTreeNode_Str("editor transform"))
		{
			drawTransform(data->editorPos, data->editorRot, data->editorTransform);
			igTreePop();
		}

		if (igTreeNode_Str("export transform"))
		{
			drawTransform(data->positionOffset, data->rotationOffset, 0);
			igTreePop();
		}
	}

	if (itemType == ITEM_TYPE_PANEL)
	{
		PanelDefData* data = def->data;
		igSeparatorText("Entity Item Options");
		for (int i = 0; i < 8; i++)
		{
			Prop* prop = &data->props[i];
			if (prop->mesh == 0)
				continue;
			const char* name = 0;
			switch (i)
			{
			case PANEL_ITEM_ID_BOTTOM_LEFT:
				name = "bottom left prop";
				break;
			case PANEL_ITEM_ID_BOTTOM:
				name = "bottom prop";
				break;
			case PANEL_ITEM_ID_BOTTOM_RIGHT:
				name = "bottom right prop";
				break;
			case PANEL_ITEM_ID_LEFT:
				name = "left prop";
				break;
			case PANEL_ITEM_ID_RIGHT:
				name = "right prop";
				break;
			case PANEL_ITEM_ID_TOP_LEFT:
				name = "top left prop";
				break;
			case PANEL_ITEM_ID_TOP:
				name = "top prop";
				break;
			case PANEL_ITEM_ID_TOP_RIGHT:
				name = "top right prop";
				break;
			}
			if (igTreeNode_Str(name))
			{
				if (igTreeNode_Str("editor transform"))
				{
					drawTransform(prop->editorPos, prop->editorRot, prop->editorTransform);
					igTreePop();
				}
				if (igTreeNode_Str("export transform"))
				{
					drawTransform(prop->pos, prop->rot, prop->transform);
					igTreePop();
				}
				igTreePop();
			}
		}
	}

	igEnd();
}
