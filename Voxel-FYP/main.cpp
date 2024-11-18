#include "Vulkan_Wrapper.h"
#include "GLFW_Wrapper.h"
#include "Window.h"

#include <iostream>

int main()
{
	GlfwWrapper glfw{};

	std::unique_ptr<Window> window = glfw.CreateWindow(800, 600, "VulkanFYP");

	int temp;
	while (true)
	{
		std::cin >> temp;
		if (temp == 5)
			break;
	}
}