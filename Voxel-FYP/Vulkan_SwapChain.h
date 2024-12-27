#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <vector>

class Vulkan_Device;
class Vulkan_Surface;

class Vulkan_SwapChain
{
public:
	Vulkan_SwapChain(const std::unique_ptr<Vulkan_Device>& device, const std::unique_ptr<Vulkan_Surface>& surface, vk::Extent2D surfaceExtent, const Vulkan_SwapChain* oldSwapchain);

    const vk::Format GetImageFormat() const { return m_imageFormat; }
private:
	vk::raii::SwapchainKHR m_swapChain;
    std::vector<vk::Image> m_images;
    std::vector<vk::ImageView> m_imageViews;
    vk::Format m_imageFormat;
    vk::Extent2D m_imageExtent;

	struct CreateInfo;
	CreateInfo GetCreateInfo(const std::unique_ptr<Vulkan_Device>& device, const std::unique_ptr<Vulkan_Surface>& surface, vk::Extent2D surfaceExtent, const Vulkan_SwapChain* oldSwapchain);

    /// <summary>
    /// Chooses the best image format for the swapchain out of those available
    /// </summary>
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;

    /// <summary>
    /// Chooses the best present mode for the swapchain out of those available
    /// </summary>
    vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;

    /// <summary>
    /// Calculates the swapchain extent that best fits framebuffer but still abides by capabilities
    /// </summary>
    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D windowExtent) const;
};

