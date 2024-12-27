#include "Vulkan_RenderPass.h"
#include "Vulkan_Device.h"

Vulkan_RenderPass::Vulkan_RenderPass(const std::unique_ptr<Vulkan_Device>& device, const vk::Format swapChainImageFormat) :
	m_renderPass{ CreateRenderPass(device, swapChainImageFormat) }
{

}

vk::raii::RenderPass Vulkan_RenderPass::CreateRenderPass(const std::unique_ptr<Vulkan_Device>& device, const vk::Format swapChainImageFormat) {
	vk::AttachmentDescription colourAttachment = {
		{}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
	};

	vk::AttachmentReference colourAttachmentRef = { 0, vk::ImageLayout::eColorAttachmentOptimal	};

	vk::SubpassDescription subpass = { {}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colourAttachmentRef };

	vk::RenderPassCreateInfo createInfo = {
		{},
		1, &colourAttachment,
		1, &subpass
	};

	return device->GetHandle().createRenderPass(createInfo);
}
