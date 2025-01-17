#include "Vulkan_DescriptorSets.h"
#include "Vulkan_Device.h"
#include "Vulkan_Buffer.h"
#include "Structures.h"

// pass vector of descriptor set layouts and also vector of counts for each descriptor set
Vulkan_DescriptorSets::Vulkan_DescriptorSets(DevicePtr device, const std::vector<vk::DescriptorSetLayoutBinding>& layouts,  uint32_t count) :
	m_descriptorSetLayout{CreateDescriptorSetLayout(device, layouts)},
	m_descriptorPool{CreateDescriptorPool(device, layouts, count)},
	m_descriptorSets{AllocateDescriptorSets(device, count)}
{
	
}

const vk::raii::DescriptorSet& Vulkan_DescriptorSets::GetDesciptorSet(uint32_t index)
{
	return m_descriptorSets[index];
}

vk::raii::DescriptorSetLayout Vulkan_DescriptorSets::CreateDescriptorSetLayout(DevicePtr device, const std::vector<vk::DescriptorSetLayoutBinding>& layouts)
{
	vk::DescriptorSetLayoutCreateInfo createInfo{
		{}, //flags
		layouts
	};

	return device->GetHandle().createDescriptorSetLayout(createInfo);
}

vk::raii::DescriptorPool Vulkan_DescriptorSets::CreateDescriptorPool(DevicePtr device, const std::vector<vk::DescriptorSetLayoutBinding>& layouts, uint32_t count)
{
	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.reserve(layouts.size());
	for(const auto& layout : layouts)
	{
		// TODO: for same descriptor types just double pool size
		vk::DescriptorPoolSize poolSize{
			layout.descriptorType,
			static_cast<uint32_t>(count)
		};
		poolSizes.emplace_back(poolSize);
	}

	vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

	vk::DescriptorPoolCreateInfo createInfo{
		flags,
		count, // max sets
		poolSizes
	};

	return device->GetHandle().createDescriptorPool(createInfo);
}

vk::raii::DescriptorSets Vulkan_DescriptorSets::AllocateDescriptorSets(DevicePtr device, uint32_t count)
{
	std::vector<vk::DescriptorSetLayout> layouts(count, m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo{
		m_descriptorPool,
		layouts
	};

	return vk::raii::DescriptorSets(device->GetHandle(), allocInfo);
}
