#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <vector>

class GLFW_Window;

#include "Vulkan_FWD.h"

class Vulkan_Wrapper
{
public:
	Vulkan_Wrapper(GLFW_Window* window, bool validationEnabled = false);
	~Vulkan_Wrapper(); // must be included in CPP to use fwd declare in unique_ptr

	bool IsValidationEnabled() const { return m_validationLayersEnabled; }
	GLFW_Window* GetWindow() const { return m_window; }
	const vk::raii::Context& GetContext() const { return m_ctx; }
	const RendererPtr GetRenderer() const { return m_renderer; }

	std::vector<const char*> GetValidationLayers() const;
	void RecreateSwapChain();
private:
	const bool m_validationLayersEnabled;

	vk::raii::Context m_ctx;
	std::unique_ptr<Vulkan_Instance> m_instance;
	std::unique_ptr<Vulkan_Debugger> m_debugMessenger;
	std::unique_ptr<Vulkan_Surface> m_surface;
	std::unique_ptr<Vulkan_Device> m_device;
	std::unique_ptr<Vulkan_SwapChain> m_swapChain;
	std::unique_ptr<Vulkan_RenderPass> m_renderPass;
	std::unique_ptr<Vulkan_Image> m_depthImage;
	std::unique_ptr<Vulkan_Pipeline> m_pipeline;

	std::unique_ptr<Vulkan_CommandPool> m_graphicsPool;
	std::unique_ptr<Vulkan_DescriptorSets> m_descriptorSets;
	std::unique_ptr<Vulkan_Renderer> m_renderer;

	GLFW_Window* m_window;
};