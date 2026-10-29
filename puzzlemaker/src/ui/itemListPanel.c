#include "antline/antline.h"
#include "dynList.h"
#include <stdbool.h>
#include <stdio.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "item/item.h"

#include "cimgui.h"
#include "save.h"
#include "selection.h"
#include "ui/itemPanel.h"

extern Antline* antlines;
extern unsigned int iconTexture;

static char buf[64];
void itemListRender()
{
	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;

	ImGuiStyle* style = igGetStyle();
	ImVec2 buttonSize;
	buttonSize.y = igGetTextLineHeight();
	buttonSize.x = buttonSize.y * 150 / 110;

	ImVec4 tint;
	tint.x = style->Colors[ImGuiCol_WindowBg].x * 1.1;
	tint.y = style->Colors[ImGuiCol_WindowBg].y * 1.1;
	tint.z = style->Colors[ImGuiCol_WindowBg].z * 1.1;
	tint.w = style->Colors[ImGuiCol_WindowBg].w;

	igBegin("Item List", 0, 0);

	int flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;
	if (igTreeNodeEx_Str("Items", flags))
	{
		Item* items = getItemList();
		int len = dynList_size(items);

		if (igBeginTable("items", 1, ImGuiTableFlags_RowBg, zero, 0))
		{
			for (int i = 0; i < len; i++)
			{
				Item* item = &items[i];
				if (!isItemValid(item))
					continue;
				igTableNextRow(0, 0);
				igTableSetColumnIndex(0);
				igPushID_Int(i);
				snprintf(buf, 64, "%s: %d", item->def->name, item->index);
				char selected = 0;
				if (selection.type == SELECTION_ITEM)
					selected = (item == selection.item);
				if (igSelectable_Bool(buf, selected, ImGuiSelectableFlags_AllowOverlap, zero))
				{
					selection.type = SELECTION_ITEM;
					selection.item = item;
				}
				float width = igGetContentRegionAvail().x - buttonSize.x;
				igSameLine(width, -1);
				char pressed;
				igPushStyleVar_Float(ImGuiStyleVar_FrameRounding, 1);
				igPushStyleVar_Vec2(ImGuiStyleVar_FramePadding, zero);
				if (item->hidden)
				{
					ImVec2 uv0 = {0, 0.5};
					ImVec2 uv1 = {1, 0};
					ImVec4 col = {0, 0, 0, 0};
					ImVec4 tint = {1, 1, 1, 1};

					ImTextureRef_c tex;
					tex._TexID = iconTexture;
					tex._TexData = 0;

					pressed = igImageButton(" ", tex, buttonSize, uv0, uv1, col, tint);
				}
				else
				{
					ImVec2 uv0 = {0, 1};
					ImVec2 uv1 = {1, 0.5};
					ImVec4 col = {0, 0, 0, 0};
					ImVec4 tint = {1, 1, 1, 1};

					ImTextureRef_c tex;
					tex._TexID = iconTexture;
					tex._TexData = 0;

					pressed = igImageButton(" ", tex, buttonSize, uv0, uv1, col, tint);
				}
				igPopStyleVar(2);
				if (pressed)
					item->hidden = !item->hidden;
				igPopID();
			}
			igEndTable();
		}
		igTreePop();
	}

	if (igTreeNodeEx_Str("Antlines", flags))
	{
		if (igBeginTable("antlines", 1, ImGuiTableFlags_RowBg, zero, 0))
		{

			int len = dynList_size(antlines);

			for (int i = 0; i < len; i++)
			{
				Antline* antline = &antlines[i];
				if (!isAntlineValid(antline))
					continue;
				igTableNextRow(0, 0);
				igTableSetColumnIndex(0);
				igPushID_Int(i);
				snprintf(buf, 64, "Antline %d", antline->id);
				char selected = 0;
				if (selection.type == SELECTION_ANTLINE)
					selected = (antline == selection.antline);
				ImVec2 zero;
				zero.x = 0;
				zero.y = 0;
				if (igSelectable_Bool(buf, selected, 0, zero))
				{
					selection.type = SELECTION_ANTLINE;
					selection.antline = antline;
					int l = dynList_size(antline->segments);
					if (antline->hasCheck || l == 0)
						selection.antlineSeg = &antline->baseSegment;
					else
						selection.antlineSeg = &antline->segments[0];
				}
				igPopID();
			}
			igEndTable();
		}
		igTreePop();
	}
	igEnd();
}
