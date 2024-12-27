#include "Vulkan_Pipeline.h"
#include "Vulkan_Device.h"
#include "Vulkan_RenderPass.h"
#include "Structures.h"
#include <fstream>

Vulkan_Pipeline::Vulkan_Pipeline(DevicePtr device, RenderPassPtr renderPass) :
    m_pipelineLayout{device->GetHandle(), vk::PipelineLayoutCreateInfo()},
	m_pipeline{ CreateGraphicsPipeline(device, renderPass)}
{

}

vk::raii::Pipeline Vulkan_Pipeline::CreateGraphicsPipeline(DevicePtr device, RenderPassPtr renderPass) {
    auto vertShader = CreateShaderModule(device, "shaders/vert.spv");
    auto fragShader = CreateShaderModule(device, "shaders/frag.spv");

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { {
        {
            {},
            vk::ShaderStageFlagBits::eVertex,
            vertShader,
            "main"
        },
        {
            {},
            vk::ShaderStageFlagBits::eFragment,
            fragShader,
            "main"
        }
    } };

    auto bindingDesc = Vertex::GetBindingDescription();
    auto attributeDesc = Vertex::GetAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertextInputInfo = { {}, bindingDesc, attributeDesc};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = { {}, vk::PrimitiveTopology::eTriangleList, vk::False };
    vk::PipelineViewportStateCreateInfo viewportState = { {}, 1, nullptr, 1, nullptr };
    vk::PipelineMultisampleStateCreateInfo multisampler = { {}, vk::SampleCountFlagBits::e1, vk::False };

    vk::PipelineRasterizationStateCreateInfo rasteriser = { 
        {}, // flags
        vk::False, // depth clamp
        vk::False, // discard enable
        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
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
        nullptr, // depth stencil state
        &colourBlending,
        &dynamicState,
        m_pipelineLayout, renderPass->GetHandle(),
        0 // subpass
    };

    return device->GetHandle().createGraphicsPipeline(nullptr, createInfo);
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
