#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include <GLFW/glfw3.h>
#include <cimgui.h>
#include <cimgui_impl.h>

static ImGuiContext* ctx;
static ImGuiIO* io;

void initUi(GLFWwindow* window)
{
	ctx = igCreateContext(0);
	io = igGetIO_Nil();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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
}

void uiEndFrame()
{
	igRender();
	ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
