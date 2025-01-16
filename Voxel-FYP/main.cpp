#include "Vulkan_Wrapper.h"
#include "GLFW_Wrapper.h"
#include "GLFW_Window.h"
#include "Vulkan_Renderer.h"

#include <iostream>


int main()
{
	{
		GlfwWrapper glfw{};

		std::unique_ptr<GLFW_Window> window = glfw.CreateWindow(800, 600, "VulkanFYP");
		Vulkan_Wrapper vulkanWrapper{ window.get(), true };

		RendererPtr renderer = vulkanWrapper.GetRenderer();

		while (!window->ShouldClose()) 
		{
			window->PollEvents();

			try
			{
				renderer->DrawFrame();
			}
			catch (std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
				vulkanWrapper.RecreateSwapChain();
				if (window->resized)
					window->resized = false;
			}
		}
	}
	return 0;
}