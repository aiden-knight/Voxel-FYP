#include "GLFW_Window.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

GLFW_Window::GLFW_Window(const int width, const int height, const char* title)
{
	m_windowHandle = glfwCreateWindow(width, height, title, NULL, NULL);
	m_resized = false;

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

bool GLFW_Window::ShouldClose() const
{
	return glfwWindowShouldClose(m_windowHandle);
}

void GLFW_Window::PollEvents() const
{
	glfwPollEvents();
}

std::vector<const char*> GLFW_Window::GetRequiredInstanceExtensions() const
{
	uint32_t count;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	return { extensions, extensions + count };
}

void GLFW_Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	GLFW_Window* customWindow = reinterpret_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
	customWindow->m_resized = true;
}
