#include "antline/antline.h"
#include "cglm/euler.h"
#include "cglm/quat.h"
#include "cglm/vec3.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include "utils.h"
#include "voxel/voxel.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "camera.h"
#include "cimgui.h"
#include "dynList.h"
#include "grab.h"
#include "picker.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

#define PICK_OUTPUT 0
#define PICK_KV 1
#define PICK_ANTLINE 2
#define PICK_POINT_AT 3

extern Antline* antlines;

extern ImVec2 zero;

int groupCount;
int packageCount;
int queueItemAdd = 0;
int queueItemAddGrab = 0;
int queueItemAddAntline = 0;
extern ItemGroup* groups;
extern Package* packageList;
extern int mouseMode;

extern Picker picker;

static Item* pickEntity;
static Antline* pickAntline;
static Item* prevItem = 0;
static int pickItemId = 0;
static int pickType = 0;

static char buf[50];

void setSelectedItem(Item* item)
{
	selection.type = SELECTION_ITEM;
	selection.item = item;
}

void initItemPanel()
{
	groupCount = dynList_size(groups);
	packageCount = dynList_size(packageList);
}

char* outputNames = 0;

void itemPanelStub()
{
	if (igButton("Add Item", zero))
	{
		queueItemAddGrab = 0;
		queueItemAddAntline = 0;
		igOpenPopup_Str("pick", 0);
	}

	if (queueItemAdd)
	{
		queueItemAddGrab = 1;
		queueItemAdd = 0;
		igOpenPopup_Str("pick", 0);
	}
	if (igBeginPopup("pick", 0))
	{
		for (int k = 0; k < packageCount; k++)
		{
			Package* package = &packageList[k];
			if (package->enabled == 0)
				continue;
			if (!igBeginMenu(package->baseName, 1))
				continue;
			for (int i = 0; i < package->itemGroupCount; i++)
			{
				ItemGroup* group = &groups[i + package->itemGroupStart];
				if (!igBeginMenu(group->name, 1))
					continue;
				for (int j = 0; j < group->size; j++)
				{
					ItemDefinition* def = &getItemDefinitions()[j + group->startInd];
					if (igSelectable_Bool(def->name, 0, 0, zero))
					{
						clearSelection();
						vec3 offset;
						vec3 pos;
						ivec3 ipos;

						glm_vec3_scale(forward, 5, offset);

						pos[0] = offset[0] + cameraPos[0];
						pos[1] = offset[1] + cameraPos[1];
						pos[2] = offset[2] + cameraPos[2];

						ipos[0] = floorf(pos[0]);
						ipos[1] = floorf(pos[1]);
						ipos[2] = floorf(pos[2]);

						selection.type = SELECTION_ITEM;
						selection.item = addItemFromDef(def, ipos);
						updateGrab(cameraPos, forward);
						if (queueItemAddGrab)
							mouseMode = 4;
					}
				}
				igEndMenu();
			}
			igEndMenu();
		}
		if (queueItemAddAntline)
		{
			if (igSelectable_Bool("Antline", 0, 0, zero))
			{
				selection.type = SELECTION_ANTLINE;
				selection.antline = addAntline();
				selection.antlineSeg = &selection.antline->baseSegment;
				updateGrab(cameraPos, forward);
			}
		}
		igEndPopup();
	}
}

void itemPanelRender()
{
	if (selection.type == SELECTION_ITEM)
	{
		if (prevItem != selection.item)
			goto end;
		int treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
		if (igTreeNodeEx_Str("Item", treeNodeFlags))
		{
			igText("%s, %d", selection.item->def->name, selection.item->index);
			if (igButton("Remove", zero))
			{
        igTreePop();
				removeItem(selection.item);
				selection.item = 0;
				selection.type = SELECTION_NONE;
				updateCulledVoxels();
				goto end;
			}

			char updateVoxel = 0;
			if (igDragFloat3("Position", selection.item->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			{
				updateVoxel = 1;
				updateItemTransform(selection.item);
			}
			if (igDragFloat3("Rotation", selection.item->dir, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			{
				updateVoxel = 1;
				updateItemTransformRot(selection.item);
			}

			char pressed;
			if (picker.active && pickType == PICK_POINT_AT && pickItemId == -1)
				pressed = igButton("Picking Item", zero);
			else
				pressed = igButton("Point At Item", zero);

			if (pressed)
			{
				picker.active = 1;
				picker.ptr = &pickEntity;
				pickEntity = 0;
				pickType = PICK_POINT_AT;
				pickItemId = -1;
			}

			if (picker.active == 0 && pickEntity && pickType == PICK_POINT_AT && pickItemId == -1)
			{
				vec3 src;
				vec3 dest;
				memcpy(src, selection.item->pos, sizeof(vec3));
				memcpy(dest, pickEntity->pos, sizeof(vec3));
				vec3 f;
				f[0] = dest[0] - src[0];
				f[1] = dest[1] - src[1];
				f[2] = dest[2] - src[2];
				glm_normalize(f);

				vec3 angles;

				float xyDist = sqrtf(f[0] * f[0] + f[2] * f[2]);
				angles[0] = -atan2f(f[1], xyDist);
				if (xyDist > 0.001f)
					angles[1] = (atan2f(f[0], f[2]));
				else
				{
					vec3 left;
					float f2 = modff(fabsf(f[2] + 0.5f), &left[0]);
					left[0] = -f[1];
					left[1] = f[0] - f2 * f[2];
					angles[1] = atan2f(-left[0], left[1]);
				}

				angles[0] = glm_deg(angles[0]);
				angles[1] = glm_deg(angles[1]);
				angles[2] = 0.0f;
				memcpy(selection.item->dir, angles, sizeof(vec3));
				updateItemTransformRot(selection.item);

				pickEntity = 0;
			}

			if (selection.item->def->type == ITEM_TYPE_PANEL)
			{
				updateVoxel = 1;
				PanelData* data = selection.item->data;
				PanelDefData* def = selection.item->def->data;
				if (igDragFloat2("Size", data->size, 0.01f, 0.0f, 9999.0f, "%.3f", 0))
				{
					if (data->size[0] > def->maxSize[0])
						data->size[0] = def->maxSize[0];
					if (data->size[0] < def->minSize[0])
						data->size[0] = def->minSize[0];

					if (data->size[1] > def->maxSize[1])
						data->size[1] = def->maxSize[1];
					if (data->size[1] < def->minSize[1])
						data->size[1] = def->minSize[1];
				}
				if (def->horizTile)
				{
					if (igInputInt("Horizontal Tile", &data->tile[0], 1, 1, 0))
					{
						updateVoxel = 1;
						if (data->tile[0] < 1)
							data->tile[0] = 1;
					}
				}
				if (def->vertTile)
				{
					if (igInputInt("Vertical Tile", &data->tile[1], 1, 1, 0))
					{
						updateVoxel = 1;
						if (data->tile[1] < 1)
							data->tile[1] = 1;
					}
				}
			}
			if (updateVoxel)
			{
				if (selection.item->def->flags & ITEM_FLAGS_DELVOXELSEDITOR)
					updateCulledVoxels();
			}

			if (selection.item->def->type == ITEM_TYPE_VOLUME)
			{
				VolumeItemData* data = selection.item->data;
				VolumeItemDef* def = selection.item->def->data;
				if (igDragFloat3("Size", data->size, 0.01f, 0.0f, 9999.0f, "%.3f", 0))
				{
					updateVoxel = 1;
					if (data->size[0] > def->maxSize[0])
						data->size[0] = def->maxSize[0];
					if (data->size[0] < def->minSize[0])
						data->size[0] = def->minSize[0];

					if (data->size[1] > def->maxSize[1])
						data->size[1] = def->maxSize[1];
					if (data->size[1] < def->minSize[1])
						data->size[1] = def->minSize[1];

					if (data->size[2] > def->maxSize[2])
						data->size[2] = def->maxSize[2];
					if (data->size[2] < def->minSize[2])
						data->size[2] = def->minSize[2];
				}
			}
			igTreePop();
		}

		if (igTreeNodeEx_Str("Key Values", treeNodeFlags))
		{
			int l = dynList_size(selection.item->def->kvs);
			for (int i = 0; i < l; i++)
			{
				ItemKv* kv = &selection.item->kv[i];
				int type = kv->def->type & (~(TYPE_INSTANCE));
				igPushID_Int(i);
				if (type == TYPE_INT)
					igInputInt(kv->def->displayName, &kv->value.i, 1, 0, 0);
				if (type == TYPE_FLOAT)
					igInputFloat(kv->def->displayName, &kv->value.f, 1, 0, "%.2f", 0);
				if (type == TYPE_BOOL)
					igCheckbox(kv->def->displayName, (bool*)&kv->value.b);
				if (type == TYPE_STRING)
					igInputText(kv->def->displayName, kv->value.s, 256, 0, 0, 0);
				if (type == TYPE_PICKER)
				{
					char pressed;
					if (kv->value.i != -1)
					{
						char msg[32];
						Item* item = getItem(kv->value.i);
						snprintf(msg, sizeof(msg), "Entity: %s %d", item->def->name, kv->value.i);
						pressed = igButton(msg, zero);

						if (igIsItemHovered(0))
							item->highlighted = 1;
						else
							item->highlighted = 0;
					}
					else if (picker.active && pickEntity && pickItemId == i && pickType == PICK_KV)
						pressed = igButton("Entity: Picking", zero);
					else
					{
						if (kv->def->dropValues->b)
						{
							ImVec4 col = {.x = 1, .y = 0, .z = 0, .w = 1};
							igPushStyleColor_Vec4(ImGuiCol_Border, col);
						}
						pressed = igButton("Entity: None", zero);
						if (kv->def->dropValues->b)
							igPopStyleColor(1);
					}

					if (pressed)
					{
						picker.active = 1;
						picker.ptr = &pickEntity;
						pickEntity = 0;
						kv->value.i = -1;
						pickItemId = i;
						pickType = PICK_KV;
					}

					if (picker.active == 0 && pickEntity && i == pickItemId && pickType == PICK_KV)
					{
						kv->value.i = pickEntity->index;
						pickEntity = 0;
					}

					igSameLine(0, -1);
					igText("%s", kv->def->displayName);
				}

				if (type == TYPE_BITFIELD)
				{
					if (igTreeNodeEx_Str(kv->def->displayName, ImGuiTreeNodeFlags_Framed))
					{
						int len = dynList_size(kv->def->dropNames);
						for (int i = 0; i < len; i++)
							igCheckbox(kv->def->dropNames[i], (bool*)&kv->value.s[i]);
						igTreePop();
					}
				}

				if (type == TYPE_VEC3)
				{
					char pressed;
					if (picker.active && pickType == PICK_POINT_AT && pickItemId == i)
						pressed = igButton("Picking Item", zero);
					else
						pressed = igButton("Point At Item", zero);

					if (pressed)
					{
						picker.active = 1;
						picker.ptr = &pickEntity;
						pickEntity = 0;
						pickType = PICK_POINT_AT;
						pickItemId = i;
					}

					if (picker.active == 0 && pickEntity && pickType == PICK_POINT_AT && pickItemId == i)
					{
						vec3 src;
						vec3 dest;
						memcpy(src, selection.item->pos, sizeof(vec3));
						memcpy(dest, pickEntity->pos, sizeof(vec3));
						vec3 f;
						f[0] = dest[0] - src[0];
						f[1] = dest[1] - src[1];
						f[2] = dest[2] - src[2];
						glm_normalize(f);

						vec3 angles;

						float xyDist = sqrtf(f[0] * f[0] + f[2] * f[2]);
						angles[0] = -atan2f(f[1], xyDist);
						if (xyDist > 0.001f)
							angles[1] = (atan2f(f[0], f[2]));
						else
						{
							vec3 left;
							float f2 = modff(fabsf(f[2] + 0.5f), &left[0]);
							left[0] = -f[1];
							left[1] = f[0] - f2 * f[2];
							angles[1] = atan2f(-left[0], left[1]);
						}

						angles[2] = 0.0f;

						vec4 quat;
						glm_quat_identity(quat);
						if (kv->def->dropValues->i == GIZMO_ANGLE_OFFSET)
							memcpy(quat, selection.item->quat, sizeof(vec4));
						if (kv->def->dropValues->i == GIZMO_ANGLE_OFFSET_FLOOR)
						{
							vec4 quat2;
							vec3 off = {GLM_PI_2, 0, 0};
							glm_euler_yxz_quat(off, quat2);
							glm_quat_mul(selection.item->quat, quat2, quat);
						}
						if (kv->def->dropValues->i == GIZMO_ANGLE_OFFSET_CEILING)
						{
							vec4 quat2;
							vec3 off = {-GLM_PI_2, 0, 0};
							glm_euler_yxz_quat(off, quat2);
							glm_quat_mul(selection.item->quat, quat2, quat);
						}

						vec4 quat2;
						glm_euler_yxz_quat(angles, quat2);
						glm_quat_inv(quat, quat);
						glm_quat_mul(quat, quat2, quat);
						mat4 m;
						glm_quat_mat4(quat, m);
						getEulerAngles(m, kv->value.v);
						kv->value.v[0] = glm_deg(kv->value.v[0]);
						kv->value.v[1] = glm_deg(kv->value.v[1]);
						kv->value.v[2] = glm_deg(kv->value.v[2]);

						pickEntity = 0;
					}
					igSameLine(0, -1);
					igDragFloat3(kv->def->displayName, kv->value.v, 0.25f, 0.0f, 0.0f, "%.2f", 0);
				}

				if (type & TYPE_DROPDOWN)
				{
					if (igBeginCombo(kv->def->displayName, kv->def->dropNames[kv->value.i], 0))
					{
						int len = dynList_size(kv->def->dropNames);
						for (int i = 0; i < len; i++)
						{
							char selected = (kv->value.i == i);
							if (igSelectable_Bool(kv->def->dropNames[i], selected, 0, zero))
								kv->value.i = i;
							if (selected)
								igSetItemDefaultFocus();
						}
						igEndCombo();
					}
				}
				igPopID();
			}
			igTreePop();
		}

		int l = dynList_size(selection.item->outputs);
		int defCount = dynList_size(selection.item->def->outputs);
		if (igTreeNodeEx_Str("Outputs", treeNodeFlags))
		{
			if (defCount > 0)
			{
				if (igButton("Add Output", zero))
				{
					dynList_resize((void**)&selection.item->outputs, l + 1);
					ItemOutput* output = &selection.item->outputs[l];
					output->entity = -1;
					output->def = selection.item->def->outputs;
					output->input = 0;
					output->antline = -1;
					output->inverted = 0;
					output->onlyPrimary = 0;
				}
			}

			ItemOutput* outputs = selection.item->outputs;
			for (long long i = 0; i < l; i++)
			{
				ItemOutput* output = &outputs[i];
				if (igTreeNodeEx_Ptr((void*)i, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesFull,
									 output->def->name))
				{
					if (igButton("Remove", zero))
					{
						for (long long j = i; j < l - 1; j++)
							memcpy(&outputs[j], &outputs[j + 1], sizeof(ItemOutput));
						l--;
						dynList_resize((void**)&outputs, l);
					}

					if (igBeginCombo("Output", output->def->name, 0))
					{
						int len = dynList_size(selection.item->def->outputs);
						for (int i = 0; i < len; i++)
						{
							OutputDef* def = &selection.item->def->outputs[i];
							char selected = (output->def == def);
							if (igSelectable_Bool(def->name, selected, 0, zero))
								output->def = def;
							if (selected)
								igSetItemDefaultFocus();
						}
						igEndCombo();
					}

					char pressed;
					if (output->entity != -1)
					{
						char msg[32];
						Item* item = getItem(output->entity);
						snprintf(msg, sizeof(msg), "Entity: %s %d", item->def->name, output->entity);
						pressed = igButton(msg, zero);

						if (igIsItemHovered(0))
							item->highlighted = 1;
						else
							item->highlighted = 0;
					}
					else if (picker.active == PICKER_ITEM)
						pressed = igButton("Entity: Picking", zero);
					else
						pressed = igButton("Entity: None", zero);

					if (pressed)
					{
						picker.active = 1;
						picker.ptr = &pickEntity;
						pickEntity = 0;
						output->entity = -1;
						output->input = 0;
						pickItemId = i;
						pickType = PICK_OUTPUT;
					}

					if (picker.active == 0 && pickEntity && i == pickItemId && pickType == PICK_OUTPUT)
					{
						output->entity = pickEntity->index;
						pickEntity = 0;
					}

					igBeginDisabled(output->entity == -1);
					const char* inputName = "None";
					if (output->entity != -1)
					{
						Item* entity = getItem(output->entity);
						if (output->input)
							inputName = output->input->name;
					}
					if (igBeginCombo("Input", inputName, 0))
					{
						Item* entity = getItem(output->entity);
						InputDef* inputs = entity->def->inputs;
						int len = dynList_size(inputs);
						for (int i = 0; i < len; i++)
						{
							InputDef* def = &inputs[i];
							char selected = (output->input == def);
							if (igSelectable_Bool(def->name, selected, 0, zero))
								output->input = def;
							if (selected)
								igSetItemDefaultFocus();
						}
						igEndCombo();
					}

					pressed = 0;
					if (output->antline != -1)
					{
						char msg[32];
						Antline* antline = &antlines[output->antline];
						snprintf(msg, sizeof(msg), "Antline: Antline %d", output->antline);
						pressed = igButton(msg, zero);

						if (igIsItemHovered(0))
							antline->hovered = 1;
						else
							antline->hovered = 0;
					}
					else if (picker.active == PICKER_ANTLINE)
						pressed = igButton("Antline: Picking", zero);
					else
						pressed = igButton("Antline: None", zero);

					if (pressed)
					{
						picker.active = PICKER_ANTLINE;
						picker.ant = &pickAntline;
						pickAntline = 0;
						output->antline = -1;
						pickItemId = i;
						pickType = PICK_ANTLINE;
					}

					if (picker.active == 0 && pickAntline && i == pickItemId && pickType == PICK_ANTLINE)
					{
						output->antline = pickAntline->id;
						pickAntline = 0;
					}

					igCheckbox("Inverted", (bool*)&output->inverted);
					igCheckbox("Only Primary Output", (bool*)&output->onlyPrimary);
					igSetItemTooltip("Determines if the item should only output on the true output\n"
									 "Usefull to make buttons open a door but not close it when they are unpressed");
					igEndDisabled();

					igTreePop();
				}
			}
			igTreePop();
		}
	}
end:
	prevItem = selection.item;
}
