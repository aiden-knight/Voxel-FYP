#include "GLFW_Wrapper.h"
#include "GLFW_Window.h"

#include <GLFW/glfw3.h>

GlfwWrapper::GlfwWrapper()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

GlfwWrapper::~GlfwWrapper()
{
	glfwTerminate();
}

std::unique_ptr<Window> GlfwWrapper::CreateWindow(const int width, const int height, const char* title) const
{
	return std::unique_ptr<Window>(new GLFW_Window(width, height, title));
}
