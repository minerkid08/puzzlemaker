#include "cglm/mat4.h"
#include "item/entityItem.h"
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

static bool open = 0;

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
			char update = 0;
			update |= igDragFloat3("position", data->editorPos, 0.25, -9999, 9999, "%.3f", 0);
			update |= igDragFloat3("rotation", data->editorRot, 0.25, -9999, 9999, "%.3f", 0);
			if (update)
			{
				vec3 pos;
				vec3 rot;
				mat4 transform;
				glm_mat4_identity(transform);
				memcpy(pos, data->editorPos, sizeof(vec3));
				memcpy(rot, data->editorRot, sizeof(vec3));

				mat4 rotMat;
				vec4 quat;
				rot[0] = glm_rad(rot[0]);
				rot[1] = glm_rad(rot[1]);
				rot[2] = glm_rad(rot[2]);
				glm_translate(transform, pos);
				glm_euler_yzx_quat(rot, quat);
				glm_quat_mat4(quat, rotMat);
				glm_mat4_mul(transform, rotMat, transform);
				memcpy(data->editorTransform, transform, sizeof(mat4));
			}
			igTreePop();
		}
		if (igTreeNode_Str("export transform"))
		{
			igDragFloat3("position", data->positionOffset, 0.25, -9999, 9999, "%.3f", 0);
			igDragFloat3("rotation", data->rotationOffset, 0.25, -9999, 9999, "%.3f", 0);
			igTreePop();
		}
	}

	igEnd();
}
