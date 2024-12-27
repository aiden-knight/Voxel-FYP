#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>

#include "Vulkan_FWD.h"

class Vulkan_Buffer 
{
public:
	Vulkan_Buffer(DevicePtr device, vk::DeviceSize size, vk::BufferUsageFlags flags, vk::MemoryPropertyFlags properties);

	const vk::raii::Buffer& GetHandle() const { return m_buffer; }

	void FillBuffer(const void* inputData);
private:
	vk::raii::Buffer m_buffer;
	vk::raii::DeviceMemory m_bufferMemory;

	vk::raii::Buffer CreateBuffer(DevicePtr device, vk::DeviceSize size, vk::BufferUsageFlags flags) const;
	vk::raii::DeviceMemory AllocateMemory(DevicePtr device, vk::MemoryPropertyFlags properties) const;
};

