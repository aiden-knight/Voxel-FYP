#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>

class Vulkan_Device;
class Vulkan_RenderPass;

class Vulkan_Pipeline 
{
public:
	Vulkan_Pipeline(const std::unique_ptr<Vulkan_Device>& device, const std::unique_ptr<Vulkan_RenderPass>& renderPass);

private:
	vk::raii::PipelineLayout m_pipelineLayout;
	vk::raii::Pipeline m_pipeline;

	vk::raii::Pipeline CreateGraphicsPipeline(const std::unique_ptr<Vulkan_Device>& device, const std::unique_ptr<Vulkan_RenderPass>& renderPass);

	vk::raii::ShaderModule CreateShaderModule(const std::unique_ptr<Vulkan_Device>& device, const std::string& fileName);

	static std::vector<char> ReadFile(const std::string& fileName);
};

