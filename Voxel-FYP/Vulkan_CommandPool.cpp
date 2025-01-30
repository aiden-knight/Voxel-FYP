#include "Vulkan_CommandPool.h"

Vulkan_CommandPool::Vulkan_CommandPool(DevicePtr device, QueueType poolType) :
	m_poolType{ poolType },
	m_deviceOwnerRef{device},
	m_commandPool{CreateCommandPool()}
{}

vk::raii::CommandBuffer Vulkan_CommandPool::BeginSingleTimeCommands() const {
	vk::CommandBufferAllocateInfo allocInfo{
		m_commandPool,
		vk::CommandBufferLevel::ePrimary,
		1 // buffer count
	};

	vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(m_deviceOwnerRef->GetHandle(), allocInfo)[0]);

	vk::CommandBufferBeginInfo beginInfo{
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};
	commandBuffer.begin(beginInfo);
	return commandBuffer;
}

void Vulkan_CommandPool::EndSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const 
{
	commandBuffer.end();

	std::array<vk::CommandBuffer, 1> buffer{ commandBuffer };

	std::array<vk::SubmitInfo, 1> submitInfo{ {{
		{},			// wait semaphore
		{},			// wait dest stage
		buffer
	}}};

	vk::raii::Queue queue = m_deviceOwnerRef->GetQueue(m_poolType);
	queue.submit(submitInfo);
	queue.waitIdle(); // use fence for multiple transfers
}

vk::raii::CommandBuffers Vulkan_CommandPool::CreateCommandBuffers(const uint32_t count) const {
	vk::CommandBufferAllocateInfo allocInfo{
		m_commandPool,
		vk::CommandBufferLevel::ePrimary,
		count
	};

	vk::raii::CommandBuffers commandBuffers{ m_deviceOwnerRef->GetHandle(), allocInfo };
	return commandBuffers;
}

vk::raii::CommandPool Vulkan_CommandPool::CreateCommandPool() const
{
	vk::CommandPoolCreateInfo createInfo{
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		m_deviceOwnerRef->GetQueueIndex(m_poolType)
	};

	return m_deviceOwnerRef->GetHandle().createCommandPool(createInfo);
}
