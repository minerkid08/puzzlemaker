#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <GLFW/glfw3.h>
#include <cimgui.h>

void ImGui_ImplGlfw_InitForOpenGL(GLFWwindow*, char);
void ImGui_ImplGlfw_Shutdown();
void ImGui_ImplGlfw_NewFrame();

void ImGui_ImplOpenGL3_Init(const char*);
void ImGui_ImplOpenGL3_Shutdown();
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData*);
void ImGui_ImplOpenGL3_NewFrame();

static ImGuiContext* ctx;
static ImGuiIO* io;

void initUi(GLFWwindow* window)
{
	ctx = igCreateContext(0);
	io = igGetIO();

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
