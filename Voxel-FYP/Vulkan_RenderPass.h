#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>

class Vulkan_Device;

class Vulkan_RenderPass 
{
public:
	Vulkan_RenderPass(const std::unique_ptr<Vulkan_Device>& device, const vk::Format swapChainImageFormat);

	const vk::raii::RenderPass& GetHandle() const { return m_renderPass; }
private:
	vk::raii::RenderPass m_renderPass;

	vk::raii::RenderPass CreateRenderPass(const std::unique_ptr<Vulkan_Device>& device, const vk::Format swapChainImageFormat);
};

