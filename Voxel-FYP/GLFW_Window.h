#pragma once
#include "Window.h"

struct GLFWwindow;

class GLFW_Window : public Window
{
public:
	GLFW_Window(const int width, const int height, const char* title);
	~GLFW_Window();

private:
	GLFWwindow* m_windowHandle;

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};

