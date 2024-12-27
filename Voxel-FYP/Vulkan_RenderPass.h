#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>

#include "Vulkan_FWD.h"

class Vulkan_RenderPass 
{
public:
	Vulkan_RenderPass(DevicePtr device, const vk::Format swapChainImageFormat);

	const vk::raii::RenderPass& GetHandle() const { return m_renderPass; }
private:
	vk::raii::RenderPass m_renderPass;

	vk::raii::RenderPass CreateRenderPass(DevicePtr device, const vk::Format swapChainImageFormat);
};

