#include "Vulkan_DescriptorSets.h"
#include "Vulkan_Device.h"
#include "Vulkan_Buffer.h"
#include "Structures.h"

Vulkan_DescriptorSets::Vulkan_DescriptorSets(DevicePtr device, uint32_t count) :
	m_descriptorSetLayout{CreateDescriptorSetLayout(device)},
	m_descriptorPool{CreateDescriptorPool(device, count)},
	m_descriptorSets{AllocateDescriptorSets(device, count)}
{
	
}

const vk::raii::DescriptorSet& Vulkan_DescriptorSets::GetDesciptorSet(uint32_t index)
{
	return m_descriptorSets[index];
}

void Vulkan_DescriptorSets::UpdateDescriptorSets(DevicePtr device, const std::vector<std::pair<Vulkan_Buffer, void*>>& uniformBuffers)
{
	for (size_t i = 0; i < uniformBuffers.size(); ++i)
	{
		std::vector<vk::DescriptorBufferInfo> bufferInfo{ {{
			uniformBuffers[i].first.GetHandle(),
			0,
			sizeof(UniformBufferObject)
		}} };

		std::vector<vk::WriteDescriptorSet> descriptorWrites{ {{
			m_descriptorSets[i],
			0, 0, // dst binding and dst array element
			vk::DescriptorType::eUniformBuffer,
			{},
			bufferInfo
		}} };

		device->GetHandle().updateDescriptorSets(descriptorWrites, {});
	}
}

vk::raii::DescriptorSetLayout Vulkan_DescriptorSets::CreateDescriptorSetLayout(DevicePtr device)
{
	std::vector<vk::DescriptorSetLayoutBinding> uboBinding{ {
		{
			0, // binding
			vk::DescriptorType::eUniformBuffer,
			1, // descripter count
			vk::ShaderStageFlagBits::eVertex
		}
	} };

	vk::DescriptorSetLayoutCreateInfo createInfo{
		{}, //flags
		uboBinding
	};

	return device->GetHandle().createDescriptorSetLayout(createInfo);
}

vk::raii::DescriptorPool Vulkan_DescriptorSets::CreateDescriptorPool(DevicePtr device, uint32_t count)
{
	std::vector<vk::DescriptorPoolSize> poolSizes{ {
		{
			vk::DescriptorType::eUniformBuffer,
			static_cast<uint32_t>(count)
		}
	} };

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
