#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include "Vulkan_FWD.h"

class Vulkan_DescriptorSets
{
public:
	Vulkan_DescriptorSets(DevicePtr device, uint32_t count);

	const vk::raii::DescriptorSetLayout& GetLayout() const { return m_descriptorSetLayout; }

	const vk::raii::DescriptorSet& GetDesciptorSet(uint32_t index);
	void UpdateDescriptorSets(DevicePtr device, const std::vector<std::pair<Vulkan_Buffer, void*>>& uniformBuffers);
private:
	vk::raii::DescriptorSetLayout m_descriptorSetLayout;
	vk::raii::DescriptorPool m_descriptorPool;
	vk::raii::DescriptorSets m_descriptorSets;

	vk::raii::DescriptorSetLayout CreateDescriptorSetLayout(DevicePtr device);
	vk::raii::DescriptorPool CreateDescriptorPool(DevicePtr device, uint32_t count);
	vk::raii::DescriptorSets AllocateDescriptorSets(DevicePtr device, uint32_t count);
};

