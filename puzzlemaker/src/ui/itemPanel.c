#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item.h"

#include "cimgui.h"
#include "dynList.h"
#include "save.h"
#include "ui/itemPanel.h"

const char** itemNames;

int defCount;

extern Item** pickPtr;

static char buf[50];
static char filename[50];

void openCompilePopup(const char* filename);

void initItemPanel()
{
	filename[0] = 0;
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

	igInputText("filename", filename, 50, 0, 0, 0);

	if (igButton("save", zero))
		save(filename);
	igSameLine(0, -1);
	if (igButton("load", zero))
		load(filename);
	igSameLine(0, -1);
	if (igButton("compile", zero))
	{
		exportMap(filename);
		openCompilePopup(filename);
	}

	updateCompilePopup();

	igSeparatorText("items");

	if (igButton("add", zero))
		igOpenPopup_Str("pick", 0);

	if (igBeginPopup("pick", 0))
	{
		for (int i = 0; i < defCount; i++)
		{
			if (igSelectable_Bool(itemNames[i], 0, 0, zero))
				addItem(i);
		}
		igEndPopup();
	}

	Item* currentItem = getSelectedItem();

	if (currentItem)
	{
		igText("%s, %d", currentItem->def->name, currentItem->index);
		if (igButton("remove", zero))
		{
			removeItem(currentItem);
			goto end;
		}

		if (igDragFloat3("position", currentItem->pos, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			updateItemTransform(currentItem);
		if (igDragFloat3("rotation", currentItem->dir, 0.01f, 0.0f, 0.0f, "%.3f", 0))
			updateItemTransform(currentItem);

		igSeparatorText("kvs");

		int l = dynList_size(currentItem->def->kvs);
		for (int i = 0; i < l; i++)
		{
			ItemKv* kv = &currentItem->kv[i];
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

		l = dynList_size(currentItem->outputs);

		int defCount = dynList_size(currentItem->def->outputs);
		if (defCount == 0)
			igText("no outputs for this item");
		else
		{
			if (igButton("add output", zero))
			{
				dynList_resize((void**)&currentItem->outputs, l + 1);
				ItemOutput* output = &currentItem->outputs[l];
				output->entity = 0;
				output->def = currentItem->def->outputs;
				output->input = 0;
				output->inverted = 0;
			}

			for (long long i = 0; i < l; i++)
			{
				ItemOutput* output = &currentItem->outputs[i];
				if (igTreeNode_Ptr((void*)i, output->def->name))
				{
					if (igBeginCombo("output", output->def->name, 0))
					{
						int len = dynList_size(currentItem->def->outputs);
						for (int i = 0; i < len; i++)
						{
							OutputDef* def = &currentItem->def->outputs[i];
							char selected = (output->def == def);
							if (igSelectable_Bool(def->name, selected, 0, zero))
								output->def = def;
							if (selected)
								igSetItemDefaultFocus();
						}
						igEndCombo();
					}

					char pressed;
					if (output->entity != 0)
						pressed = igButton("entity: something", zero);
					else if (pickPtr)
						pressed = igButton("entity: picking", zero);
					else
						pressed = igButton("entity: none", zero);

					if (pressed)
						pickPtr = &output->entity;

					if (output->entity)
					{
						const char* inputName = (output->input == 0 ? "none" : output->input->name);
						if (igBeginCombo("input", inputName, 0))
						{
							InputDef* inputs = output->entity->def->inputs;
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
