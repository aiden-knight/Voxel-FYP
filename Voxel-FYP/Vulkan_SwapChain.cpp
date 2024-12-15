#include "Vulkan_SwapChain.h"
#include "Vulkan_Device.h"
#include "Vulkan_Surface.h"

#include <algorithm>

struct Vulkan_SwapChain::CreateInfo
{
	vk::SwapchainCreateInfoKHR createInfo;
};

Vulkan_SwapChain::Vulkan_SwapChain(const std::unique_ptr<Vulkan_Device>& device, const std::unique_ptr<Vulkan_Surface>& surface, vk::Extent2D surfaceExtent, const Vulkan_SwapChain* oldSwapchain) :
	m_swapChain{ device->GetHandle(), GetCreateInfo(device, surface, surfaceExtent, oldSwapchain).createInfo }

{

}

Vulkan_SwapChain::CreateInfo Vulkan_SwapChain::GetCreateInfo(const std::unique_ptr<Vulkan_Device>& device, const std::unique_ptr<Vulkan_Surface>& surface, vk::Extent2D surfaceExtent, const Vulkan_SwapChain* oldSwapchain)
{
	CreateInfo createInfo;

	const SwapChainSupportDetails supportDetails = device->GetSwapChainSupportDetails();
	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(supportDetails.formats);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(supportDetails.presentModes);
	vk::Extent2D extent = ChooseSwapExtent(supportDetails.capabilities, surfaceExtent);

	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
	if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
		imageCount = supportDetails.capabilities.maxImageCount;
	}

	createInfo.createInfo = {
		{},
		surface->GetHandle(),
		imageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive, 0, nullptr,
		vk::SurfaceTransformFlagBitsKHR::eIdentity,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		presentMode,
		vk::True
	};

	if (oldSwapchain)
		createInfo.createInfo.oldSwapchain = oldSwapchain->m_swapChain;

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
