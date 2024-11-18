#include "Vulkan_Wrapper.h"
#include "GLFW_Wrapper.h"
#include "Window.h"

#include <iostream>

int main()
{
	GlfwWrapper glfw{};

	std::unique_ptr<Window> window = glfw.CreateWindow(800, 600, "VulkanFYP");
	Vulkan_Wrapper vulkanWrapper{ window.get(), true };

	while (!window->ShouldClose())
	{
		window->PollEvents();


	}
}