#pragma once
#include <vector>

struct GLFWwindow;

class GLFW_Window
{
public:
	GLFW_Window(const int width, const int height, const char* title);
	~GLFW_Window();

	bool ShouldClose() const;
	void PollEvents() const;
	void GetFramebufferSize(int* width, int* height);
	std::vector<const char*> GetRequiredInstanceExtensions() const;

	GLFWwindow* GetHandle() const { return m_windowHandle; }

private:
	bool m_resized;
	GLFWwindow* m_windowHandle;

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};

