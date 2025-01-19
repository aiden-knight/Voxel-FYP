#include "Vulkan_SwapChain.h"
#include "Vulkan_Device.h"
#include "Vulkan_Surface.h"
#include "Vulkan_Image.h"
#include "Vulkan_RenderPass.h"

#include <algorithm>

Vulkan_SwapChain::Vulkan_SwapChain(DevicePtr device, SurfacePtr surface, vk::Extent2D surfaceExtent, const Vulkan_SwapChain* oldSwapchain) :
	m_imageExtent{ ChooseSwapExtent(device->GetSwapChainSupportDetails().capabilities, surfaceExtent) },
	m_imageFormat{ ChooseSwapSurfaceFormat(device->GetSwapChainSupportDetails().formats).format },
	m_swapChain{ device->GetHandle(), GetCreateInfo(device, surface, oldSwapchain) }
{
	m_images = m_swapChain.getImages();

	m_imageViews.reserve(m_images.size());
	for (const auto& image : m_images) 
	{
		vk::ImageViewCreateInfo createInfo{
			{}, // flags
			image,
			vk::ImageViewType::e2D,
			m_imageFormat,
			{}, // component mapping
			vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1
			)
		};

		m_imageViews.emplace_back(device->GetHandle().createImageView(createInfo));
	}
}

void Vulkan_SwapChain::CreateFramebuffers(DevicePtr device, RenderPassPtr renderPass, ImagePtr depthImage)
{
	m_frameBuffers.reserve(m_imageViews.size());

	for (const auto& imageView : m_imageViews) 
	{
		std::vector<vk::ImageView> attachments = { imageView, depthImage->GetImageView() };

		vk::FramebufferCreateInfo createInfo{
			{}, // flags
			renderPass->GetHandle(),
			attachments,
			m_imageExtent.width, m_imageExtent.height,
			1 // layers
		};
		m_frameBuffers.emplace_back(device->GetHandle().createFramebuffer(createInfo));
	}
}

void Vulkan_SwapChain::CreateImGuiFramebuffers(DevicePtr device, RenderPassPtr renderPass)
{
	m_imGuiFrameBuffers.reserve(m_imageViews.size());

	for (const auto& imageView : m_imageViews)
	{
		std::vector<vk::ImageView> attachments = { imageView };

		vk::FramebufferCreateInfo createInfo{
			{}, // flags
			renderPass->GetHandle(),
			attachments,
			m_imageExtent.width, m_imageExtent.height,
			1 // layers
		};
		m_imGuiFrameBuffers.emplace_back(device->GetHandle().createFramebuffer(createInfo));
	}
}

vk::SwapchainCreateInfoKHR Vulkan_SwapChain::GetCreateInfo(DevicePtr device, SurfacePtr surface, const Vulkan_SwapChain* oldSwapchain) const
{
	const SwapChainSupportDetails supportDetails = device->GetSwapChainSupportDetails();
	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(supportDetails.formats);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(supportDetails.presentModes);

	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
	if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
		imageCount = supportDetails.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = {
		{},
		surface->GetHandle(),
		imageCount,
		m_imageFormat,
		surfaceFormat.colorSpace,
		m_imageExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive, 0, nullptr,
		vk::SurfaceTransformFlagBitsKHR::eIdentity,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		presentMode,
		vk::True
	};

	if (oldSwapchain)
		createInfo.oldSwapchain = oldSwapchain->m_swapChain;

	return createInfo;
}

vk::SurfaceFormatKHR Vulkan_SwapChain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	return availableFormats.front();
}

vk::PresentModeKHR Vulkan_SwapChain::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			return availablePresentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Vulkan_SwapChain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D windowExtent) const
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		vk::Extent2D extent{
			std::clamp(windowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp(windowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};

		return extent;
	}
}
