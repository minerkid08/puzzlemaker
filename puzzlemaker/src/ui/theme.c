#include "cjson.h"
#include "error.h"
#include "jsonUtils.h"
#include <stdlib.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include <cimgui.h>

typedef struct
{
	const char* name;
	int value;
} ThemeElement;

int charToInt(int c)
{
	switch (c)
	{
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'A':
		return 10;
	case 'B':
		return 11;
	case 'C':
		return 12;
	case 'D':
		return 13;
	case 'E':
		return 14;
	case 'F':
		return 15;
	case 'a':
		return 10;
	case 'b':
		return 11;
	case 'c':
		return 12;
	case 'd':
		return 13;
	case 'e':
		return 14;
	case 'f':
		return 15;
	}
	return 0;
}

static const ThemeElement styleMap[] = {{"Alpha", ImGuiStyleVar_Alpha},
										{"DisabledAlpha", ImGuiStyleVar_DisabledAlpha},
										{"WindowPadding", ImGuiStyleVar_WindowPadding},
										{"WindowRounding", ImGuiStyleVar_WindowRounding},
										{"WindowBorderSize", ImGuiStyleVar_WindowBorderSize},
										{"WindowMinSize", ImGuiStyleVar_WindowMinSize},
										{"WindowTitleAlign", ImGuiStyleVar_WindowTitleAlign},
										{"ChildRounding", ImGuiStyleVar_ChildRounding},
										{"ChildBorderSize", ImGuiStyleVar_ChildBorderSize},
										{"PopupRounding", ImGuiStyleVar_PopupRounding},
										{"PopupBorderSize", ImGuiStyleVar_PopupBorderSize},
										{"FramePadding", ImGuiStyleVar_FramePadding},
										{"FrameRounding", ImGuiStyleVar_FrameRounding},
										{"FrameBorderSize", ImGuiStyleVar_FrameBorderSize},
										{"ItemSpacing", ImGuiStyleVar_ItemSpacing},
										{"ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing},
										{"IndentSpacing", ImGuiStyleVar_IndentSpacing},
										{"CellPadding", ImGuiStyleVar_CellPadding},
										{"ScrollbarSize", ImGuiStyleVar_ScrollbarSize},
										{"ScrollbarRounding", ImGuiStyleVar_ScrollbarRounding},
										{"GrabMinSize", ImGuiStyleVar_GrabMinSize},
										{"GrabRounding", ImGuiStyleVar_GrabRounding},
										{"TabRounding", ImGuiStyleVar_TabRounding},
										{"ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign},
										{"SelectableTextAlign", ImGuiStyleVar_SelectableTextAlign},
										{"SeparatorTextBorderSize", ImGuiStyleVar_SeparatorTextBorderSize},
										{"SeparatorTextAlign", ImGuiStyleVar_SeparatorTextAlign},
										{"SeparatorTextPadding", ImGuiStyleVar_SeparatorTextPadding},
										{0, 0}};

static const ThemeElement colorMap[] = {{"Text", ImGuiCol_Text},
										{"TextDisabled", ImGuiCol_TextDisabled},
										{"WindowBg", ImGuiCol_WindowBg},
										{"ChildBg", ImGuiCol_ChildBg},
										{"PopupBg", ImGuiCol_PopupBg},
										{"Border", ImGuiCol_Border},
										{"BorderShadow", ImGuiCol_BorderShadow},
										{"FrameBg", ImGuiCol_FrameBg},
										{"FrameBgHovered", ImGuiCol_FrameBgHovered},
										{"FrameBgActive", ImGuiCol_FrameBgActive},
										{"TitleBg", ImGuiCol_TitleBg},
										{"TitleBgActive", ImGuiCol_TitleBgActive},
										{"TitleBgCollapsed", ImGuiCol_TitleBgCollapsed},
										{"MenuBarBg", ImGuiCol_MenuBarBg},
										{"ScrollbarBg", ImGuiCol_ScrollbarBg},
										{"ScrollbarGrab", ImGuiCol_ScrollbarGrab},
										{"ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered},
										{"ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive},
										{"CheckMark", ImGuiCol_CheckMark},
										{"SliderGrab", ImGuiCol_SliderGrab},
										{"SliderGrabActive", ImGuiCol_SliderGrabActive},
										{"Button", ImGuiCol_Button},
										{"ButtonHovered", ImGuiCol_ButtonHovered},
										{"ButtonActive", ImGuiCol_ButtonActive},
										{"Header", ImGuiCol_Header},
										{"HeaderHovered", ImGuiCol_HeaderHovered},
										{"HeaderActive", ImGuiCol_HeaderActive},
										{"Separator", ImGuiCol_Separator},
										{"SeparatorHovered", ImGuiCol_SeparatorHovered},
										{"SeparatorActive", ImGuiCol_SeparatorActive},
										{"ResizeGrip", ImGuiCol_ResizeGrip},
										{"ResizeGripHovered", ImGuiCol_ResizeGripHovered},
										{"ResizeGripActive", ImGuiCol_ResizeGripActive},
										{"Tab", ImGuiCol_Tab},
										{"TabHovered", ImGuiCol_TabHovered},
										{"TabActive", ImGuiCol_TabSelected},
										{"TabActiveOverline", ImGuiCol_TabSelectedOverline},
										{"TabUnfocused", ImGuiCol_TabDimmed},
										{"TabUnfocusedActive", ImGuiCol_TabDimmedSelected},
										{"TabUnfocusedActiveOverline", ImGuiCol_TabDimmedSelectedOverline},
										{"PlotLines", ImGuiCol_PlotLines},
										{"PlotLinesHovered", ImGuiCol_PlotLinesHovered},
										{"PlotHistogram", ImGuiCol_PlotHistogram},
										{"PlotHistogramHovered", ImGuiCol_PlotHistogramHovered},
										{"TableHeaderBg", ImGuiCol_TableHeaderBg},
										{"TableBorderStrong", ImGuiCol_TableBorderStrong},
										{"TableBorderLight", ImGuiCol_TableBorderLight},
										{"TableRowBg", ImGuiCol_TableRowBg},
										{"TableRowBgAlt", ImGuiCol_TableRowBgAlt},
										{"TextSelectedBg", ImGuiCol_TextSelectedBg},
										{"DragDropTarget", ImGuiCol_DragDropTarget},
										{"NavHighlight", ImGuiCol_NavCursor},
										{"NavWindowingHighlight", ImGuiCol_NavWindowingHighlight},
										{"NavWindowingDimBg", ImGuiCol_NavWindowingDimBg},
										{"ModalWindowDimBg", ImGuiCol_ModalWindowDimBg},
										{0, 0}};

void loadImguiTheme()
{
	FILE* file = fopen("imguiTheme.json", "rb");
	if (file == 0)
		return;

	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(len + 1);

	fread(data, 1, len, file);
	data[len] = 0;

	int res;
	initErrorStackD("editorSettings.json", res);
	if (res)
		startErrorLoop();

	cJSON* json = cJSON_Parse(data);
	if (json == 0)
		jsonParseError(data, cJSON_GetErrorPtr(), "editorSettings.json");
	free(data);

	const char* font = jsonGetStrC(json, "font", 0);
	if (font)
	{
		float fontSize = jsonGetFloatC(json, "fontSize", 16);
		ImGuiIO* io = igGetIO_Nil();
		io->FontDefault = ImFontAtlas_AddFontFromFileTTF(io->Fonts, font, fontSize, 0, 0);
	}

	ImGuiStyle* igstyle = igGetStyle();

	cJSON* colors = jsonGetObjectC(json, "colors");
	if (colors)
	{
		int i = 0;
		while (1)
		{
			const ThemeElement* elem = &colorMap[i];
			if (elem->name == 0)
				break;
			cJSON* item = cJSON_GetObjectItem(colors, elem->name);
			i++;
			if (item == 0)
				continue;

			pushStackFrame(elem->name);

			if (cJSON_IsString(item))
			{
				const char* v = item->valuestring;
				if (strlen(v) < 7)
					errorf("string must be at least 7 chars long\n");

				printf("color %s is string\n", elem->name);

				unsigned int col = strtoul(v + 1, 0, 16);

				ImVec4 vec;

				vec.x = charToInt(v[1]);
				vec.x *= 16;
				vec.x += charToInt(v[2]);
				vec.y = charToInt(v[3]);
				vec.y *= 16;
				vec.y += charToInt(v[4]);
				vec.z = charToInt(v[5]);
				vec.z *= 16;
				vec.z += charToInt(v[6]);
				if (strlen(v) > 7)
				{
					vec.w = charToInt(v[7]);
					vec.w *= 16;
					vec.w += charToInt(v[8]);
				}
				else
					vec.w = 255;

				vec.x /= 255;
				vec.y /= 255;
				vec.z /= 255;
				vec.w /= 255;

				igPushStyleColor_Vec4(elem->value, vec);
				continue;
			}

			if (!cJSON_IsArray(item))
				errorf("bad value for key '%s' type must be vec4");

			ImVec4 v;
			v.x = jsonArrGetFloat(item, 0);
			v.y = jsonArrGetFloat(item, 1);
			v.z = jsonArrGetFloat(item, 2);
			v.w = jsonArrGetFloat(item, 3);
			igPushStyleColor_Vec4(elem->value, v);
			popStackFrame();
		}
		popStackFrame();
	}

	cJSON* style = jsonGetObjectC(json, "style");
	if (style)
	{
		int i = 0;
		while (1)
		{
			const ThemeElement* elem = &styleMap[i];
			if (elem->name == 0)
				break;
			cJSON* item = cJSON_GetObjectItem(style, elem->name);
			i++;
			if (item == 0)
				continue;

			if (cJSON_IsNumber(item))
			{
				float v = cJSON_GetNumberValue(item);
				igPushStyleVar_Float(elem->value, v);
				continue;
			}

			if (!cJSON_IsArray(item))
				errorf("bad value for key '%s' type must be vec4 or number");

			pushStackFrame(elem->name);

			ImVec2 v;
			v.x = jsonArrGetFloat(item, 0);
			v.y = jsonArrGetFloat(item, 1);
			igPushStyleVar_Vec2(elem->value, v);
			popStackFrame();
		}
		popStackFrame();
	}

	cJSON_Delete(json);
}
