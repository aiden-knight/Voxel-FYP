#include "Vulkan_Wrapper.h"
#include "GLFW_Wrapper.h"
#include "GLFW_Window.h"
#include "Vulkan_Renderer.h"
#include "Simulator.h"

#include <iostream>
#include <chrono>

float GetDeltaTime()
{
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();

	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;
	return deltaTime;
}

int main()
{
	{
		GlfwWrapper glfw{};

		std::unique_ptr<GLFW_Window> window = glfw.CreateWindow(800, 600, "VulkanFYP");
		Vulkan_Wrapper vulkanWrapper{ window.get(), true };

		RendererPtr renderer = vulkanWrapper.GetRenderer();
		Simulator simulator{ renderer->GetParticleVector() };

		while (!window->ShouldClose()) 
		{
			window->PollEvents();

			simulator.Update(GetDeltaTime());

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