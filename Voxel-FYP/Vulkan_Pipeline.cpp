#include "Vulkan_Pipeline.h"
#include "Vulkan_Device.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_DescriptorSets.h"
#include "Structures.h"
#include <fstream>

Vulkan_Pipeline::Vulkan_Pipeline(DevicePtr device, DescriptorSetsPtr descriptorSets, RenderPassPtr renderPass) :
    m_pipelineLayout{CreatePipelineLayout(device, descriptorSets)},
	m_pipeline{ CreateGraphicsPipeline(device, renderPass)}
{

}

Vulkan_Pipeline::Vulkan_Pipeline(DevicePtr device, DescriptorSetsPtr descriptorSets, const std::string& computeShaderFileName) :
    m_pipelineLayout{ CreatePipelineLayout(device, descriptorSets) },
    m_pipeline{ CreateComputePipeline(device, computeShaderFileName) }
{
}

vk::raii::PipelineLayout Vulkan_Pipeline::CreatePipelineLayout(DevicePtr device, DescriptorSetsPtr descriptorSets)
{
    std::vector<vk::DescriptorSetLayout> layouts {descriptorSets->GetLayout() };
    vk::PipelineLayoutCreateInfo createInfo{
        {},layouts
    };
    return device->GetHandle().createPipelineLayout(createInfo);
}

vk::raii::Pipeline Vulkan_Pipeline::CreateGraphicsPipeline(DevicePtr device, RenderPassPtr renderPass) {
    auto vertShader = CreateShaderModule(device, "shaders/shader.vert.spv");
    auto geomShader = CreateShaderModule(device, "shaders/shader.geom.spv");
    auto fragShader = CreateShaderModule(device, "shaders/shader.frag.spv");

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { {
        {
            {},
            vk::ShaderStageFlagBits::eVertex,
            vertShader,
            "main"
        },
        {
            {},
            vk::ShaderStageFlagBits::eGeometry,
            geomShader,
            "main"
        },
        {
            {},
            vk::ShaderStageFlagBits::eFragment,
            fragShader,
            "main"
        }
    } };

    auto bindingDesc = Particle::GetBindingDescription();
    auto attributeDesc = Particle::GetAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertextInputInfo = { {}, bindingDesc, attributeDesc};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = { {}, vk::PrimitiveTopology::ePointList, vk::False };
    vk::PipelineViewportStateCreateInfo viewportState = { {}, 1, nullptr, 1, nullptr };
    vk::PipelineMultisampleStateCreateInfo multisampler = { {}, vk::SampleCountFlagBits::e1, vk::False };

    vk::PipelineRasterizationStateCreateInfo rasteriser = { 
        {}, // flags
        vk::False, // depth clamp
        vk::False, // discard enable
        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
        vk::False, 0.0f, 0.0f, 0.0f,// depth bias
        1.0f // line width
    };
    
    vk::PipelineColorBlendAttachmentState colourBlendAttachment = {
        vk::False, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo colourBlending = { {}, vk::False, vk::LogicOp::eCopy, 1, &colourBlendAttachment };

    std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState = { {}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data() };

    vk::PipelineDepthStencilStateCreateInfo depthStencilState{ {},
        vk::True, vk::True, // depthTestEnable, depthWriteEnable
        vk::CompareOp::eLess,
        vk::False, vk::False // depthBoundsTestEnable, stencilTestEnable
    };

    vk::GraphicsPipelineCreateInfo createInfo = {
        {}, // flags
        static_cast<uint32_t>(shaderStages.size()),
        shaderStages.data(),
        &vertextInputInfo,
        &inputAssembly,
        nullptr, // tessellation state
        &viewportState,
        &rasteriser,
        &multisampler,
        &depthStencilState,
        &colourBlending,
        &dynamicState,
        m_pipelineLayout, renderPass->GetHandle(),
        0 // subpass
    };

    return device->GetHandle().createGraphicsPipeline(nullptr, createInfo);
}

vk::raii::Pipeline Vulkan_Pipeline::CreateComputePipeline(DevicePtr device, const std::string& computeShaderFileName)
{
    vk::raii::ShaderModule compShader = CreateShaderModule(device, computeShaderFileName);

    vk::PipelineShaderStageCreateInfo computeShaderStage = { 
        {}, vk::ShaderStageFlagBits::eCompute, compShader, "main" 
    };

    vk::ComputePipelineCreateInfo createInfo{
        {},
        computeShaderStage,
        m_pipelineLayout
    };

    return device->GetHandle().createComputePipeline(nullptr, createInfo);
}

vk::raii::ShaderModule Vulkan_Pipeline::CreateShaderModule(DevicePtr device, const std::string& fileName) {
    auto shaderCode = ReadFile(fileName);

    vk::ShaderModuleCreateInfo createInfo{
        {},
        shaderCode.size(),
        reinterpret_cast<const uint32_t*>(shaderCode.data())
    };

    return device->GetHandle().createShaderModule(createInfo);
}

std::vector<char> Vulkan_Pipeline::ReadFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
