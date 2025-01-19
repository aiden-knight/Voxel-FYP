#include "Vulkan_RenderPass.h"
#include "Vulkan_Device.h"

Vulkan_RenderPass::Vulkan_RenderPass(DevicePtr device, const vk::Format swapChainImageFormat, bool imGui) :
	m_renderPass{ imGui ? CreateImGuiRenderPass(device, swapChainImageFormat) : CreateRenderPass(device, swapChainImageFormat)}
{

}

vk::raii::RenderPass Vulkan_RenderPass::CreateRenderPass(DevicePtr device, const vk::Format swapChainImageFormat) {
	std::vector<vk::AttachmentDescription> attachments = { {
		{
			{}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
		},
		{
			{}, device->FindDepthFormat(), vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
		}
	} };

	std::array<vk::AttachmentReference, 1> colourAttachmentRef{ {{ 0, vk::ImageLayout::eColorAttachmentOptimal }} };
	vk::AttachmentReference depthAttachmentRef{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

	std::array<vk::SubpassDescription, 1> subpass{ { {
		{}, vk::PipelineBindPoint::eGraphics, {}, colourAttachmentRef, {}, &depthAttachmentRef
	}} };

	std::array<vk::SubpassDependency, 1> dependencies{ {{
		vk::SubpassExternal,								0, // source and dest subpass
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,	// src stage
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,	// dst stage
		vk::AccessFlagBits::eNone, // src access mask
		vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite // dst access mask
	}} };

	vk::RenderPassCreateInfo createInfo = {
		{},
		attachments,
		subpass,
		dependencies
	};

	return device->GetHandle().createRenderPass(createInfo);
}

vk::raii::RenderPass Vulkan_RenderPass::CreateImGuiRenderPass(DevicePtr device, const vk::Format swapChainImageFormat)
{
	std::vector<vk::AttachmentDescription> attachments = { {
		{
			{}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR
		}
	} };

	std::array<vk::AttachmentReference, 1> colourAttachmentRef{ {{ 0, vk::ImageLayout::eColorAttachmentOptimal }} };

	std::array<vk::SubpassDescription, 1> subpass{ { {
		{}, vk::PipelineBindPoint::eGraphics, {}, colourAttachmentRef
	}} };

	std::array<vk::SubpassDependency, 1> dependencies{ {{
		vk::SubpassExternal,								0, // source and dest subpass
		vk::PipelineStageFlagBits::eColorAttachmentOutput ,	// src stage
		vk::PipelineStageFlagBits::eColorAttachmentOutput,	// dst stage
		vk::AccessFlagBits::eColorAttachmentWrite, // src access mask
		vk::AccessFlagBits::eColorAttachmentWrite  // dst access mask
	}} };

	vk::RenderPassCreateInfo createInfo = {
		{},
		attachments,
		subpass,
		dependencies
	};

	return device->GetHandle().createRenderPass(createInfo);
}
