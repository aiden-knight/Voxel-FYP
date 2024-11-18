#include "GLFW_Window.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

GLFW_Window::GLFW_Window(const int width, const int height, const char* title)
{
	m_windowHandle = glfwCreateWindow(width, height, title, NULL, NULL);

	glfwSetWindowUserPointer(m_windowHandle, this);
	glfwSetFramebufferSizeCallback(m_windowHandle, FramebufferResizeCallback);
}

GLFW_Window::~GLFW_Window()
{
	if (m_windowHandle)
	{
		glfwDestroyWindow(m_windowHandle);
		m_windowHandle = nullptr;
	}
}

void GLFW_Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	GLFW_Window* customWindow = reinterpret_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
	customWindow->m_resized = true;
}
