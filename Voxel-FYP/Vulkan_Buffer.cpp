#include "Vulkan_Buffer.h"
#include "Vulkan_Device.h"
#include "Vulkan_CommandPool.h"

Vulkan_Buffer::Vulkan_Buffer(DevicePtr device, vk::DeviceSize size, vk::BufferUsageFlags flags, vk::MemoryPropertyFlags properties) :
	m_buffer{CreateBuffer(device, size, flags)},
	m_bufferMemory{AllocateMemory(device, properties)}
{
	m_buffer.bindMemory(m_bufferMemory, 0);
}

void Vulkan_Buffer::FillBuffer(const void* inputData) 
{
	vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();

	void* data = m_bufferMemory.mapMemory(0, memRequirements.size);
	std::memcpy(data, inputData, static_cast<size_t>(memRequirements.size));
	m_bufferMemory.unmapMemory();
}

void Vulkan_Buffer::CopyFromBuffer(CommandPoolPtr commandPool, Vulkan_Buffer& source, vk::DeviceSize size)
{
	auto commandBuffer = commandPool->BeginSingleTimeCommands();

	std::array<vk::BufferCopy, 1> regions{ { {0, 0, size} } };
	commandBuffer.copyBuffer(source.m_buffer, m_buffer, regions);
	commandPool->EndSingleTimeCommands(std::move(commandBuffer));
}

vk::raii::Buffer Vulkan_Buffer::CreateBuffer(DevicePtr device, vk::DeviceSize size, vk::BufferUsageFlags flags) const 
{
	vk::BufferCreateInfo createInfo{
		{},
		size,
		flags,
		vk::SharingMode::eExclusive
	};

	return device->GetHandle().createBuffer(createInfo);
}

vk::raii::DeviceMemory Vulkan_Buffer::AllocateMemory(DevicePtr device, vk::MemoryPropertyFlags properties) const 
{
	vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();
	
	vk::MemoryAllocateInfo allocInfo{
		memRequirements.size,
		device->FindMemoryType(memRequirements.memoryTypeBits, properties)
	};

	return device->GetHandle().allocateMemory(allocInfo);
}
