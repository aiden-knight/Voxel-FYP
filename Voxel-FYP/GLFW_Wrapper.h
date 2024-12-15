#pragma once
#include <memory>

class GLFW_Window;

class GlfwWrapper
{
public:
	GlfwWrapper();
	~GlfwWrapper();

	std::unique_ptr<GLFW_Window> CreateWindow(const int width, const int height, const char* title) const;
};