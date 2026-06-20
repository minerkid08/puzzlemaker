#include "camera.h"
#include "renderer/framebuffer.h"
#include "ui/fileBrowser.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include <GLFW/glfw3.h>
#include <cimgui.h>
#include <cimgui_impl.h>
#include "save.h"
#include "ui/itemPanel.h"

static ImGuiContext* ctx;
static ImGuiIO* io;

float uiScale = 1.0f;

void initUi(GLFWwindow* window)
{
	ctx = igCreateContext(0);
	io = igGetIO_Nil();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGuiStyle* style = igGetStyle();
  ImGuiStyle_ScaleAllSizes(style, uiScale);
  style->FontScaleMain = uiScale;

	const char* glslVersion = "#version 330 core";
	ImGui_ImplGlfw_InitForOpenGL(window, 1);
	ImGui_ImplOpenGL3_Init(glslVersion);
	igStyleColorsDark(0);
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

extern char mouseOffsetX;
extern char mouseOffsetY;

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

void export2(const char* name);

void openCompilePopup(const char* filename);

void uiMenuBar()
{
	ImVec2 zero;
	zero.x = 0;
	zero.y = 0;
  igBeginMenuBar();
	if (igMenuItem_Bool("save", 0, 0, 1))
		fileBrowserSave();
	if (igMenuItem_Bool("save as", 0, 0, 1))
    fileBrowserOpen(MODE_SAVE);
	if (igMenuItem_Bool("load", 0, 0, 1))
    fileBrowserOpen(MODE_LOAD);
	if (igMenuItem_Bool("compile", 0, 0, 1))
	{
    char* path = fileBrowserGetPath();
		export2(path);
		openCompilePopup(path + 5);
	}

	updateCompilePopup();
  igEndMenuBar();
}
