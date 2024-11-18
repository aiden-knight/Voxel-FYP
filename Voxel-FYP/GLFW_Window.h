#pragma once
#include "Window.h"

struct GLFWwindow;

class GLFW_Window final : public Window
{
public:
	GLFW_Window(const int width, const int height, const char* title);
	~GLFW_Window();

	bool ShouldClose() const;
	void PollEvents() const;
	WindowExtensions GetRequiredInstanceExtensions() const;

private:
	GLFWwindow* m_windowHandle;

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};

