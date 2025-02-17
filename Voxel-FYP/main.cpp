#include "Vulkan_Wrapper.h"
#include "GLFW_Wrapper.h"
#include "GLFW_Window.h"
#include "Vulkan_Renderer.h"
#include "Simulator.h"
#include "ImGuiConfig.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

void OutputPerformance(const std::vector<float>& performance, const ProfileInfo& profileInfo)
{
	std::string fileName = "model(" + profileInfo.model + ")" +
		"_resolution(" + std::to_string(profileInfo.resolution) + ")" +
		"_voxelCount(" + std::to_string(profileInfo.voxelCount) + ")" +
		"_simulated(" + (profileInfo.wasSimulated ? "true" : "false") + ")";

	std::filesystem::create_directory("data");
	std::ofstream outFile{ "./data/" + fileName + ".csv", std::ofstream::out | std::ofstream::trunc };
	if (outFile.good())
	{
		for (auto it = performance.begin() + 1; it != performance.end(); ++it)
		{
			outFile << *it << std::endl;
		}
	}
	outFile.close();
}

void Profile(const float& deltaTime)
{
	ImGuiConfig* config = ImGuiConfig::GetInstance();

	static std::array<float, 60> lastTimes;
	static unsigned int counter = 0;
	static std::vector<float> performanceData;

	lastTimes[counter++] = deltaTime;
	if (counter == lastTimes.size())
	{
		float sum = 0;
		for (const float& time : lastTimes)
		{
			sum += time;
		}
		sum /= lastTimes.size();

		if (performanceData.size() == 0)
			performanceData.reserve(config->profileInfo.dataCount + 1);

		performanceData.push_back(sum);
		if (performanceData.size() == config->profileInfo.dataCount + 1)
		{
			config->profile = false;
			OutputPerformance(performanceData, config->profileInfo);
			performanceData.clear();
		}

		counter = 0;
	}
}

float GetDeltaTime()
{
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();

	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;

	if (ImGuiConfig::GetInstance()->profile)
	{
		Profile(deltaTime);
	}

	return deltaTime;
}

int main()
{
	{
		GlfwWrapper glfw{};

		std::unique_ptr<GLFW_Window> window = glfw.CreateWindow(800, 600, "VulkanFYP");

		bool validationEnabled = false;
#ifndef NDEBUG
		validationEnabled = true;
#endif

		Vulkan_Wrapper vulkanWrapper{ window.get(), validationEnabled };

		RendererPtr renderer = vulkanWrapper.GetRenderer();
		Simulator simulator{ renderer->GetVoxelisation() };

		while (!window->ShouldClose()) 
		{
			window->PollEvents();

			simulator.Update(GetDeltaTime());

			renderer->DrawFrame();
		}
	}
	return 0;
}