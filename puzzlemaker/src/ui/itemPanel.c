#include "antline/antline.h"
#include "item/panel.h"
#include "item/volumeItem.h"
#include <stdbool.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "camera.h"
#include "cimgui.h"
#include "dynList.h"
#include "picker.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

#define PICK_OUTPUT 0
#define PICK_KV 1
#define PICK_ANTLINE 2

extern Antline* antlines;

extern ImVec2 zero;

int groupCount;
extern ItemGroup* groups;

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
}

char* outputNames = 0;

void itemPanelStub()
{
	if (igButton("add item", zero))
		igOpenPopup_Str("pick", 0);

	if (igBeginPopup("pick", 0))
	{
		for (int i = 0; i < groupCount; i++)
		{
			ItemGroup* group = &groups[i];
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
				}
			}
			igEndMenu();
		}
		igEndPopup();
	}
}

void itemPanelRender()
{
	igSeparatorText("items");

	if (selection.type == SELECTION_ITEM)
	{
		if (prevItem != selection.item)
			goto end;
		igText("%s, %d", selection.item->def->name, selection.item->index);
		if (igButton("remove", zero))
		{
			removeItem(selection.item);
			selection.item = 0;
			selection.type = SELECTION_NONE;
			goto end;
		}

		if (igDragFloat3("position", selection.item->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			updateItemTransform(selection.item);
		if (igDragFloat3("rotation", selection.item->dir, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			updateItemTransformRot(selection.item);

		if (selection.item->def->type == ITEM_TYPE_PANEL)
		{
			PanelData* data = selection.item->data;
			PanelDefData* def = selection.item->def->data;
			if (igDragFloat2("size", data->size, 0.01f, 0.0f, 9999.0f, "%.3f", 0))
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
				if (igInputInt("horiz tile", &data->tile[0], 1, 1, 0))
				{
					if (data->tile[0] < 1)
						data->tile[0] = 1;
				}
			}
			if (def->vertTile)
			{
				if (igInputInt("vert tile", &data->tile[1], 1, 1, 0))
				{
					if (data->tile[1] < 1)
						data->tile[1] = 1;
				}
			}
		}

		if (selection.item->def->type == ITEM_TYPE_VOLUME)
		{
			VolumeItemData* data = selection.item->data;
			VolumeItemDef* def = selection.item->def->data;
			if (igDragFloat3("size", data->size, 0.01f, 0.0f, 9999.0f, "%.3f", 0))
			{
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

		igSeparatorText("kvs");

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
					snprintf(msg, sizeof(msg), "entity: %s %d", item->def->name, kv->value.i);
					pressed = igButton(msg, zero);

					if (igIsItemHovered(0))
						item->highlighted = 1;
					else
						item->highlighted = 0;
				}
				else if (picker.active)
					pressed = igButton("entity: picking", zero);
				else
					pressed = igButton("entity: none", zero);

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

		igSeparatorText("outputs");

		l = dynList_size(selection.item->outputs);
		int defCount = dynList_size(selection.item->def->outputs);
		if (defCount == 0)
			igText("no outputs for this item");
		else
		{
			if (igButton("add output", zero))
			{
				dynList_resize((void**)&selection.item->outputs, l + 1);
				ItemOutput* output = &selection.item->outputs[l];
				output->entity = -1;
				output->def = selection.item->def->outputs;
				output->input = 0;
				output->antline = -1;
				output->inverted = 0;
			}

			ItemOutput* outputs = selection.item->outputs;
			for (long long i = 0; i < l; i++)
			{
				ItemOutput* output = &outputs[i];
				if (igTreeNodeEx_Ptr((void*)i, ImGuiTreeNodeFlags_DefaultOpen, output->def->name))
				{
					if (igButton("remove", zero))
					{
						for (long long j = i; j < l - 1; j++)
							memcpy(&outputs[j], &outputs[j + 1], sizeof(ItemOutput));
						l--;
						dynList_resize((void**)&outputs, l);
					}

					if (igBeginCombo("output", output->def->name, 0))
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
						snprintf(msg, sizeof(msg), "entity: %s %d", item->def->name, output->entity);
						pressed = igButton(msg, zero);

						if (igIsItemHovered(0))
							item->highlighted = 1;
						else
							item->highlighted = 0;
					}
					else if (picker.active == PICKER_ITEM)
						pressed = igButton("entity: picking", zero);
					else
						pressed = igButton("entity: none", zero);

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
					const char* inputName = "none";
					if (output->entity != -1)
					{
						Item* entity = getItem(output->entity);
						if (output->input)
							inputName = output->input->name;
					}
					if (igBeginCombo("input", inputName, 0))
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
					igEndDisabled();

					pressed = 0;
					if (output->antline != -1)
					{
						char msg[32];
						Antline* antline = &antlines[output->antline];
						snprintf(msg, sizeof(msg), "antline: antline %d", output->antline);
						pressed = igButton(msg, zero);

						if (igIsItemHovered(0))
							antline->hovered = 1;
						else
							antline->hovered = 0;
					}
					else if (picker.active == PICKER_ANTLINE)
						pressed = igButton("antline: picking", zero);
					else
						pressed = igButton("antline: none", zero);

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

					igCheckbox("inverted", (bool*)&output->inverted);

					igTreePop();
				}
			}
		}
	}
end:
	prevItem = selection.item;
}
