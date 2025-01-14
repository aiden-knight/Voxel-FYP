#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>

#include "Vulkan_FWD.h"

class Vulkan_Pipeline 
{
public:
	Vulkan_Pipeline(DevicePtr device, RenderPassPtr renderPass, DescriptorSetsPtr descriptorSets);

	const vk::raii::Pipeline& GetHandle() const { return m_pipeline; }
	const vk::raii::PipelineLayout& GetLayout() const { return m_pipelineLayout; }
	
private:
	vk::raii::PipelineLayout m_pipelineLayout;
	vk::raii::Pipeline m_pipeline;

	vk::raii::PipelineLayout CreatePipelineLayout(DevicePtr device, DescriptorSetsPtr descriptorSets);
	vk::raii::Pipeline CreateGraphicsPipeline(DevicePtr device, RenderPassPtr renderPass);

	vk::raii::ShaderModule CreateShaderModule(DevicePtr device, const std::string& fileName);

	static std::vector<char> ReadFile(const std::string& fileName);
};

