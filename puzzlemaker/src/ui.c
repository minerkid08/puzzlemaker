#include "camera.h"
#include "mapsettings.h"
#include "renderer/framebuffer.h"
#include "renderer/texture.h"
#include "ui/fileBrowser.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include "compile/compile.h"
#include "save.h"
#include "ui/itemPanel.h"
#include <GLFW/glfw3.h>
#include <cimgui.h>
#include <cimgui_impl.h>

static ImGuiContext* ctx;
static ImGuiIO* io;

float uiScale = 1.0f;

char queueCompile = 0;
ImVec2 zero;

unsigned int iconTexture = 0;

void loadImguiTheme();
void initUi(GLFWwindow* window)
{
	iconTexture = loadTexture(0, "icons.png", 0);

	zero.x = 0;
	zero.y = 0;

	ctx = igCreateContext(0);
	io = igGetIO_Nil();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	igStyleColorsDark(0);
	loadImguiTheme();

	ImGuiStyle* style = igGetStyle();
	ImGuiStyle_ScaleAllSizes(style, uiScale);

	const char* glslVersion = "#version 330 core";
	ImGui_ImplGlfw_InitForOpenGL(window, 1);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

void uiNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	igNewFrame();

	static int dockspaceFlags = ImGuiDockNodeFlags_None;
	int windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = igGetMainViewport();
	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;
	igSetNextWindowPos(viewport->WorkPos, 0.0, zero);
	igSetNextWindowSize(viewport->WorkSize, 0.0);
	igSetNextWindowViewport(viewport->ID);

	igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
	igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);

	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
				   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	igBegin("dockspace", 0, windowFlags);
	igPopStyleVar(2);

	ImGuiIO* io = igGetIO_Nil();
	if (io->ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = igGetID_Str("dockspace");
		igDockSpace(dockspace_id, zero, dockspaceFlags, 0);
	}
}

void uiEndFrame()
{
	igEnd();
	ImGuiIO* io = igGetIO_Nil();

	igRender();
	ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* ctx = glfwGetCurrentContext();
		igUpdatePlatformWindows();
		igRenderPlatformWindowsDefault(0, 0);
		glfwMakeContextCurrent(ctx);
	}
}

extern char viewportHovered;

extern unsigned int mouseOffsetX;
extern unsigned int mouseOffsetY;

extern int width;
extern int height;

void uiViewport(FrameBuffer* framebuffer)
{
	igBegin("Viewport", 0, 0);
	viewportHovered = igIsWindowHovered(0);
	ImVec2 viewportPanelSize = igGetContentRegionAvail();

	char drawImage = 1;
	if (framebuffer->width != viewportPanelSize.x || framebuffer->height != viewportPanelSize.y)
	{
		drawImage = 0;
		aspect = viewportPanelSize.x / viewportPanelSize.y;
		width = viewportPanelSize.x;
		height = viewportPanelSize.y;
		initCamera();
		framebufferResize(framebuffer, viewportPanelSize.x, viewportPanelSize.y);
	}

	ImVec2 screenPos = igGetCursorScreenPos();
	mouseOffsetX = screenPos.x;
	mouseOffsetY = screenPos.y;

	ImVec2 a;
	a.x = 0;
	a.y = 1;
	ImVec2 b;
	b.x = 1;
	b.y = 0;

	ImTextureRef* ref = ImTextureRef_ImTextureRef_TextureID(framebuffer->color);

	if (drawImage)
		igImage(*ref, viewportPanelSize, a, b);
	igEnd();
}

static char newMapPopup = 0;
static char styleEditor = 0;
extern char running;
extern char errorActive;

void uiMenuBar()
{
	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;
	igBeginMenuBar();
	if (igBeginMenu("File", 1))
	{
		if (igMenuItem_Bool("New", 0, 0, 1))
			newMapPopup = 1;
		if (igMenuItem_Bool("Save", 0, 0, 1))
			fileBrowserSave();
		if (igMenuItem_Bool("Save As", 0, 0, 1))
			fileBrowserOpen(MODE_SAVE);
		if (igMenuItem_Bool("Load", 0, 0, 1))
			fileBrowserOpen(MODE_LOAD);
		igEndMenu();
	}
	if (igBeginMenu("Map", 1))
	{
		if (igMenuItem_Bool("Compile", 0, 0, 1))
		{
			if (mapSettings.name[0] == 0)
				fileBrowserOpen(MODE_SAVE | QUEUE_COMPILE);
			else
				queueCompile = 1;
		}
		if (igMenuItem_Bool("Map Settings", 0, 0, 1))
			openMapSettingsUi();
		igEndMenu();
	}
	if (igBeginMenu("Window", 1))
	{
		if (igMenuItem_Bool("Editor Settings", 0, 0, 1))
			openEditorSettingsUi();
		if (igMenuItem_Bool("Package Manager", 0, 0, 1))
			openPackageManager();
		if (igMenuItem_Bool("Debug", 0, 0, 1))
			openItemDebug();
		if (igMenuItem_Bool("Style Editor", 0, 0, 1))
			styleEditor = !styleEditor;
		igEndMenu();
	}
	igSeparator();
	igText("%s", mapSettings.name);

	if (queueCompile)
	{
		queueCompile = 0;
		startCompile();
	}

	if (newMapPopup)
	{
		igOpenPopup_Str("New Map", 0);
		newMapPopup = 0;
	}

	if (igBeginPopupModal("New Map", 0, 0))
	{
		igText("You are going to clear the current map");
		igText("Are you sure?");
		if (igButton("Yes", zero))
		{
			resetMap();
			igCloseCurrentPopup();
		}
		igSameLine(0, -1);
		if (igButton("No", zero))
			igCloseCurrentPopup();
		igEndPopup();
	}

	updateCompilePopup();
	renderErrorPopup();
	igEndMenuBar();

	renderMapSettingsUi();
	renderEditorSettingsUi();
	if (running && errorActive == 0)
		packageManagerRender();
	if (styleEditor)
	{
		igBegin("Style Editor", (bool*)&styleEditor, 0);
		igShowStyleEditor(igGetStyle());
		igEnd();
	}
}
