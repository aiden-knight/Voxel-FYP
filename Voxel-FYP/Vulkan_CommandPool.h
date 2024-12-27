#pragma once
#include "Vulkan_Device.h"

class Vulkan_CommandPool 
{
public:
	Vulkan_CommandPool(const std::unique_ptr<Vulkan_Device>& device, QueueType poolType);

	vk::raii::CommandBuffer BeginSingleTimeCommands() const;
	void EndSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const;

	vk::raii::CommandBuffers CreateCommandBuffers(const uint32_t count) const;
private:
	const QueueType m_poolType;
	const std::unique_ptr<Vulkan_Device>& m_deviceOwnerRef;
	vk::raii::CommandPool m_commandPool;

	vk::raii::CommandPool CreateCommandPool() const;
};