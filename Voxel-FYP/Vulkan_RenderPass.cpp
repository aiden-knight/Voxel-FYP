#include "Vulkan_RenderPass.h"
#include "Vulkan_Device.h"

Vulkan_RenderPass::Vulkan_RenderPass(const std::unique_ptr<Vulkan_Device>& device, const vk::Format swapChainImageFormat) :
	m_renderPass{ CreateRenderPass(device, swapChainImageFormat) }
{

}

vk::raii::RenderPass Vulkan_RenderPass::CreateRenderPass(const std::unique_ptr<Vulkan_Device>& device, const vk::Format swapChainImageFormat) {
	std::array<vk::AttachmentDescription, 1> colourAttachment = { {{
		{}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
	}} };

	std::array<vk::AttachmentReference, 1> colourAttachmentRef{ {{ 0, vk::ImageLayout::eColorAttachmentOptimal }} };

	std::array<vk::SubpassDescription, 1> subpass{ { { {}, vk::PipelineBindPoint::eGraphics, nullptr, colourAttachmentRef }} };

	std::array<vk::SubpassDependency, 1> dependencies{ {{
		vk::SubpassExternal,								0, // source and dest subpass
		vk::PipelineStageFlagBits::eColorAttachmentOutput,	vk::PipelineStageFlagBits::eColorAttachmentOutput,	// stages
		vk::AccessFlagBits::eNone,							vk::AccessFlagBits::eColorAttachmentWrite			// access requirements
	}} };

	vk::RenderPassCreateInfo createInfo = {
		{},
		colourAttachment,
		subpass,
		dependencies
	};

	return device->GetHandle().createRenderPass(createInfo);
}
