#include "item/panel.h"
#include "item/volumeItem.h"
#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "camera.h"
#include "cimgui.h"
#include "dynList.h"
#include "picker.h"
#include "save.h"
#include "ui/itemPanel.h"

const char** itemNames;

int defCount;

extern Picker picker;

static Item* pickEntity;

static char buf[50];

Item* selectedItem;

void clearSelectedItem()
{
	selectedItem = 0;
}

void setSelectedItem(Item* item)
{
	selectedItem = item;
}

void initItemPanel()
{
	ItemDefinition* defs = getItemDefinitions();

	defCount = dynList_size(defs);

	itemNames = dynList_new(defCount, sizeof(const char*));

	for (int i = 0; i < defCount; i++)
		itemNames[i] = defs[i].name;
}

char* outputNames = 0;

void itemPanelRender()
{
	igBegin("items", 0, 0);
	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;

	igSeparatorText("items");

	if (igButton("add", zero))
		igOpenPopup_Str("pick", 0);

	if (igBeginPopup("pick", 0))
	{
		for (int i = 0; i < defCount; i++)
		{
			if (igSelectable_Bool(itemNames[i], 0, 0, zero))
			{
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

				addItem(i, ipos);
			}
		}
		igEndPopup();
	}

	if (selectedItem)
	{
		igText("%s, %d", selectedItem->def->name, selectedItem->index);
		if (igButton("remove", zero))
		{
			removeItem(selectedItem);
			selectedItem = 0;
			goto end;
		}

		if (igDragFloat3("position", selectedItem->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			updateItemTransform(selectedItem);
		if (igDragFloat3("rotation", selectedItem->dir, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			updateItemTransform(selectedItem);

		if (selectedItem->def->type == ITEM_TYPE_PANEL)
		{
			PanelData* data = selectedItem->data;
			PanelDefData* def = selectedItem->def->data;
      if(igDragFloat2("size", data->size, 0.01f, 0.0f, 9999.0f, "%.3f", 0))
      {
        if(data->size[0] > def->maxSize[0])
          data->size[0] = def->maxSize[0];
        if(data->size[0] < def->minSize[0])
          data->size[0] = def->minSize[0];

        if(data->size[1] > def->maxSize[1])
          data->size[1] = def->maxSize[1];
        if(data->size[1] < def->minSize[1])
          data->size[1] = def->minSize[1];
      }
		}

		if (selectedItem->def->type == ITEM_TYPE_VOLUME)
		{
			VolumeItemData* data = selectedItem->data;
			VolumeItemDef* def = selectedItem->def->data;
      if(igDragFloat3("size", data->size, 0.01f, 0.0f, 9999.0f, "%.3f", 0))
      {
        if(data->size[0] > def->maxSize[0])
          data->size[0] = def->maxSize[0];
        if(data->size[0] < def->minSize[0])
          data->size[0] = def->minSize[0];

        if(data->size[1] > def->maxSize[1])
          data->size[1] = def->maxSize[1];
        if(data->size[1] < def->minSize[1])
          data->size[1] = def->minSize[1];

        if(data->size[2] > def->maxSize[2])
          data->size[2] = def->maxSize[2];
        if(data->size[2] < def->minSize[2])
          data->size[2] = def->minSize[2];
      }
		}

		igSeparatorText("kvs");

		int l = dynList_size(selectedItem->def->kvs);
		for (int i = 0; i < l; i++)
		{
			ItemKv* kv = &selectedItem->kv[i];
			if (kv->def->type == TYPE_INT)
				igInputInt(kv->def->name, &kv->value.i, 1, 0, 0);
			if (kv->def->type == TYPE_BOOL)
				igCheckbox(kv->def->name, (bool*)&kv->value.b);

			if (kv->def->type & TYPE_DROPDOWN)
			{
				if (igBeginCombo(kv->def->name, kv->def->dropNames[kv->value.i], 0))
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
		}

		igSeparatorText("outputs");

		l = dynList_size(selectedItem->outputs);

		int defCount = dynList_size(selectedItem->def->outputs);
		if (defCount == 0)
			igText("no outputs for this item");
		else
		{
			if (igButton("add output", zero))
			{
				dynList_resize((void**)&selectedItem->outputs, l + 1);
				ItemOutput* output = &selectedItem->outputs[l];
				output->entity = -1;
				output->def = selectedItem->def->outputs;
				output->input = 0;
				output->inverted = 0;
			}

			for (long long i = 0; i < l; i++)
			{
				ItemOutput* output = &selectedItem->outputs[i];
				if (igTreeNode_Ptr((void*)i, output->def->name))
				{
					if (igBeginCombo("output", output->def->name, 0))
					{
						int len = dynList_size(selectedItem->def->outputs);
						for (int i = 0; i < len; i++)
						{
							OutputDef* def = &selectedItem->def->outputs[i];
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
						output->entity = -1;
					}

					if (picker.active == 0 && pickEntity)
					{
						output->entity = pickEntity->index;
					}

					if (output->entity != -1)
					{
						Item* entity = getItem(output->entity);
						const char* inputName = (output->input == 0 ? "none" : output->input->name);
						if (igBeginCombo("input", inputName, 0))
						{
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
					}

					igCheckbox("inverted", (bool*)&output->inverted);

					igTreePop();
				}
			}
		}
	}
end:
	igEnd();
}
