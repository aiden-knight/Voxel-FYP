#pragma once
#include <memory>

class Window;

class GlfwWrapper
{
public:
	GlfwWrapper();
	~GlfwWrapper();

	std::unique_ptr<Window> CreateWindow(const int width, const int height, const char* title) const;
};